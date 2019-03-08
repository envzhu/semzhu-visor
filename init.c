#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "malloc.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "vcpu.h"
#include "virt_mmio.h"
#include "vm.h"
#include "pcpu.h"
#include "schedule.h"
#include "hyp_mmu.h"
#include "hyp_timer.h"
#include "pmu.h"
#include "virq.h"

extern uint8_t _linux_img_start;
extern uint8_t _linux_img_end;
extern uint8_t _bcm2837_rpi_3_b_img_start;
extern uint8_t _bcm2837_rpi_3_b_img_end;

extern uint8_t _initrd_img_start;
extern uint8_t _initrd_img_end;

extern uint8_t _kozos_img_start;
extern uint8_t _kozos_img_end;

extern uint8_t _sampleos_img_start;
extern uint8_t _sampleos_img_end;

static volatile int primary_start_finished = 0;

void hyp_core_init(pcpu_t *phys_cpu){
  uint64_t hcr_el2;

  /* Init hyp config reg */
  hcr_el2 = HCR_SWIO|
            HCR_TWE|  /* Trap WFE and WFI instructions */
            HCR_AMO|  /* Trap Abort interrupt */
            HCR_IMO|  /* Trap IRQ interrupt */
            HCR_FMO|  /* Trap FIQ interrupt */
            HCR_RW;   /* In EL1 run as aarch64 */

  if(phys_cpu->scheduler->trap_sleep)
    hcr_el2 |= HCR_TWE | HCR_TWI;

  WRITE_SYSREG(HCR_EL2, hcr_el2);

  mmu_init();
}

void start_primary(uint32_t *dtb_addr){
  pcpu_t *phys_cpu = current_phys_cpu_core_init();

  INTR_DISABLE;

  log_level_set(LOG_INFO);

  log_info("Secure Hypervisor\n");
  log_info("Boot primary cpu.This cpu id is %d\n", phys_cpu->cpu_id);
  log_info("This CPU clock is %d Hz\n", phys_cpu->freq);

  mem_init();
  // hyp_timer_core_init(phys_cpu);
  timer_event_init();
  schedulers_init();
  virt_mmio_reg_reset();
  virt_device_intr_init();
  
  hyp_core_init(phys_cpu);

  mmp_t linux_mmp[] = {
      {.mem_start = 0, .mem_end = 0x7FFFF, .flag = 0},
      {.mem_start = 0x80000, .mem_end = 0x800000 - 1, .img_start = &_linux_img_start, .img_end = &_linux_img_end, .flag = MEM_VM_IMG},
      {.mem_start = 0x800000, .mem_end = 0x800000 + 0x200000 - 1, .img_start = &_bcm2837_rpi_3_b_img_start, .img_end = &_bcm2837_rpi_3_b_img_end, .flag = MEM_VM_IMG},
      {.mem_start = 0x800000 + 0x200000, .mem_end = 0x8000000 - 1, .flag = 0},
      {.mem_start = 0x8000000, .mem_end = 0x10000000 - 1, .img_start = &_initrd_img_start, .img_end = &_initrd_img_end, .flag = MEM_VM_IMG},
      //{.mem_start = 0x6000000, .mem_end = 0xFFFFFFF, .img_start = &_initrd_img_start, .img_end = &_initrd_img_end, .flag = MEM_VM_IMG}},
      // {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
  };

  mmp_t kozos_mmp[] = {
      {.mem_start = 0, .mem_end = 0xFFFFF, .img_start = &_kozos_img_start, .img_end = &_kozos_img_end, .flag = MEM_VM_IMG},
      {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
  };

  mmp_t sample_mmp[] = {
      {.mem_start = 0, .mem_end = 0x7FFFF, .flag = 0},
      {.mem_start = 0x80000, .mem_end = 0xFFFFF, .img_start = &_sampleos_img_start, .img_end = &_sampleos_img_end, .flag = MEM_VM_IMG},
      {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
  };

  /* Create vm */
  //vm_create("linux1", 1, &fcfs_scheduler, 6, 0x80000,linux_mmp, sizeof(linux_mmp)/sizeof(linux_mmp[0]), 0, VIRT_INTR_UART, VIRT_MMIO_PL011|VIRT_MMIO_AUX, 0x000fffff00000000);
  vm_create("kozos1", 1, &fcfs_scheduler, 2, 0x0000, kozos_mmp, sizeof(kozos_mmp) / sizeof(kozos_mmp[0]), 0, 0, 0, 0);
  vm_create("sample1", 1, &fcfs_scheduler, 3, 0x80000, sample_mmp, sizeof(sample_mmp)/sizeof(sample_mmp[0]), 0, 0, 0, 0);

  dump_ready_vcpu(LOG_INFO);

  dump_cpu_usage_start(phys_cpu);

  /* Wake up slave cpus */
  primary_start_finished = 1;
  asm volatile("sev");

  /* Now we are ready to run guest os.So let's go! */
  log_info("Let's Run GuestOS!\n");
  do_schedule(phys_cpu);

  /* If there is not a vcpu to do in this cpu. */
  while (phys_cpu->current_vcpu == NULL)
  {
    cpu_relax();
  }

  /* If there is a vcpu to execute in this cpu, let's run! */
  vcpu_context_switch(phys_cpu->current_vcpu);
}

void start_secondary(void){
  pcpu_t *phys_cpu = current_phys_cpu_core_init();
  
  hyp_core_init(phys_cpu);

  /* Wait for the primary cpu to be ready\n */
  INTR_ENABLE;
  while(!primary_start_finished)
    asm volatile("wfe");
  INTR_DISABLE;

  log_info("Boot slave cpu.This cpu id is %dx\n", phys_cpu->cpu_id);

  /* Now we are ready to run guest os.So let's go! */
  do_schedule(phys_cpu);

  /* If there is not a vcpu to do in this cpu. */
  while (phys_cpu->current_vcpu == NULL)
  {
    INTR_ENABLE;
    while (1)              /* For qemu */
      asm volatile("wfe"); /* make cpu sleep */
  }

  /* If there is a vcpu to do in this cpu, let's run! */
  vcpu_context_switch(phys_cpu->current_vcpu);
}
