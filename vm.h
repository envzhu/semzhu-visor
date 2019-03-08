#ifndef _VM_H_INCLUDED_
#define _VM_H_INCLUDED_

typedef struct _vm_t vm_t;

#include "typedef.h"
#include "vcpu.h"
#include "schedule.h"
#include "hyp_security.h"

typedef struct _vm_t {
  int free;
  uint8_t *phys_addr;
  char *name;
  uint32_t vcpu_num;
  vcpu_t *vcpu[4];
  scheduler_t *scheduler;
  int priority;
  phys_addr_t vttbr;
  char *hyp_msg;
  uint64_t assigned_gpio;
  struct{
    uint64_t assigned_gpu_irq;
    uint32_t fiq_index;
    uint32_t gpu_irq_route;
  }vic;
} vm_t;

typedef enum _mmp_attr_t {
  MEM = 0,
  MEM_HYP_VM_MSG,
  MEM_VM_IMG,
} mmp_attr_t;

typedef struct _mmp_t {
  phys_addr_t phys_addr;
  phys_addr_t mem_start;
  phys_addr_t mem_end;
  phys_addr_t img_start;
  phys_addr_t img_end;
  mmp_attr_t flag;
} mmp_t;

void vm_create(char *name, uint8_t vcpu_num, scheduler_t *scheduler, int priority, 
            phys_addr_t entry_addr, mmp_t *mmp, int mmp_size, 
            uint64_t sec_opt, uint64_t excl_intr_opt, uint64_t excl_mmio_opt, uint64_t assigned_gpio);

void vm_force_shutdown(vm_t *vm);

#endif
