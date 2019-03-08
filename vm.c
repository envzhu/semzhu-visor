#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "malloc.h"
#include "hyp_mmu.h"
#include "vcpu.h"
#include "schedule.h"
#include "vm.h"
#include "hyp_call.h"
#include "virt_mmio.h"
#include "schedule.h"

#define VM_NUM 10

vm_t vms[VM_NUM];
#include "virq.h"
void vm_create(char *name, uint8_t vcpu_num, scheduler_t *scheduler, int priority, 
            phys_addr_t entry_addr, mmp_t *mmp, int mmp_size, 
            uint64_t sec_opt, uint64_t excl_intr_opt, uint64_t excl_mmio_opt, uint64_t assigned_gpio){

  
  log_info("Create vm; name : %s\n", name);
  log_info("excl_intr_opt : %#x\n", VIRT_INTR_UART);

  if(vcpu_num > CPU_NUM)
    hyp_panic("Required vcpu num is too large!");
  
  /* find free a vm_t block*/
  int i;
  for(i=0; i<VM_NUM; i++){
    if(vms[i].free == 0)break;
  }
  /*if tehre isn't a free vm_t block */
  if(i == VM_NUM)
    hyp_panic("Not found a free vm_t block\n");

  vm_t *vm = &vms[i]; 

  /* TODO : VMを再利用できるようにする */
  vm->free = 1;

  vm->name = name;
  vm->vcpu_num = vcpu_num;

  vm->scheduler = scheduler;
  vm->priority = priority;

  // map pagetable
  vm->vttbr = alloc_vttbr();

  log_info("VM memory map init\n");
  for(i = 0; i < mmp_size; i++){
    log_info("mmp[%d]; mem_start : %#8x, mem_end : %#8x\n",
        i, mmp[i].mem_start, mmp[i].mem_end);
    mmp[i].phys_addr = malloc(mmp[i].mem_end + 1 - mmp[i].mem_start);
    map_page_table(vm->vttbr, mmp[i].mem_start, mmp[i].phys_addr, 
                      mmp[i].mem_end + 1 - mmp[i].mem_start);

    switch(mmp[i].flag){
      case 0:
        break;
      case MEM_VM_IMG:
        /* copy image */
        memcpy(mmp[i].phys_addr, mmp[i].img_start, (uint64_t)(mmp[i].img_end - mmp[i].img_start));
        break;
      case MEM_HYP_VM_MSG:
        vm->hyp_msg = mmp[i].phys_addr;
        map_page_table(vm->vttbr, mmp[i].mem_start, vm->hyp_msg, HYP_VM_MSG_SIZE);
        break;
      default:
        hyp_panic("Illegal flag in mmp\n");
        break;
    }
  }
  log_info("VM memory map end\n");

  /* Set Virtualization MMIO */
  vm->assigned_gpio = assigned_gpio;
  excl_mmio_assign(vm, excl_mmio_opt);
  virt_mmio_reg_assign(vm);
  log_info("excl_intr_opt : %#x\n", excl_intr_opt);
  vm->vic.assigned_gpu_irq = 0xffffffffffffffff;
  virt_device_intr_set(vm);

  log_info("generated VM name :%s,  vttbr : %#x\n", vm->name, vm->vttbr);
  /* TODO : マルチコアをサポートする */
  vm->vcpu[0] =  vcpu_create(vm, 0, vm->vttbr, vm->hyp_msg, (phys_addr_t)entry_addr);
  
  /* Add to ready que */
  vcpu_ready(vm->vcpu[0]);
}

void vm_force_shutdown(vm_t *vm){
  int i;

  if(vm == NULL)
    log_error("Cannot shutdown NULL vm\n");
  
  log_info("Shutdown vm:%s\n", vm->name);

  /* off and release each vcpu which this vm has */
  for(i=0; i < vm->vcpu_num; i++)
    vcpu_off(vm->vcpu[i]);

  /* TODO : freen vm_t block memory */
}
