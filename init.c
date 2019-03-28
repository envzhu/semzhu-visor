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

  log_info("Semzhu-Visor\n");
  log_info("Boot primary cpu.This cpu id is %d\n", phys_cpu->cpu_id);
  log_info("This CPU clock is %d Hz\n", phys_cpu->freq);

  mem_init();
  // hyp_timer_core_init(phys_cpu);
  timer_event_init();
  schedulers_init();
  virt_mmio_reg_reset();
  virt_device_intr_init();
  
  hyp_core_init(phys_cpu);

  init_vm_create();

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
