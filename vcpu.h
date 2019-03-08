#ifndef _VCPU_H_INCLUDED_
#define _VCPU_H_INCLUDED_

typedef struct _vcpu_t vcpu_t;

#include "typedef.h"
#include "pcpu.h"
#include "vm.h"
#include "hyp_security.h"

#define VCPU_NUM 10

typedef enum {
    VCPU_STATE_INIT = 0,
    VCPU_STATE_RUN,
    VCPU_STATE_READY,
    VCPU_STATE_SLEEP
} vcpu_state_t;

typedef struct _vcpu_reg_t {
  uint64_t x[32];/* $x0 ~ $x30 */
  float128_t q[32];
} vcpu_reg_t;

/*
 * TODO:
 *    - Add floating point control registers 
 *        to vcpu context  
 */

typedef struct _vcpu_sysreg_t {
  uint64_t pc;    // elr_el2
  uint64_t cpsr;  // spsr_el2
  uint64_t sp_el0;
  uint64_t sp_el1;
  uint64_t vbar_el1;
  uint64_t esr_el1;
  uint64_t elr_el1;
  uint64_t far_el1;
  uint64_t par_el1;
  uint64_t spsr_el1;
  uint64_t sctlr_el1;
  uint64_t tcr_el1;
  uint64_t ttbr0_el1;
  uint64_t ttbr1_el1;
  uint64_t mair_el1;
  uint32_t midr_el1;  //vpidr_el2
  uint64_t mpidr_el1; //vmpidr_el2 
  uint64_t cpacr_el1;
  
  uint64_t contextidr_el1;
  uint64_t tpidr_el0;
  uint64_t tpidr_el1;
  uint64_t tpidrro_el0;

  /* generic timer registers */
  uint64_t cntv_cval_el0;
  uint32_t cntv_ctl_el0;
  uint64_t cntvoff_el2;

  uint32_t cntkctl_el1;
  uint32_t cntp_ctl_el0;
	uint64_t cntp_cval_el0;
	
} vcpu_sysreg_t;

typedef struct _vcpu_t{
  vm_t *vm;
  struct _vcpu_t *next;// next ready vcpu   
  uint32_t vcpu_id; // virtial cpu core id
  pcpu_t *phys_cpu;  // physical cpu
  vcpu_state_t state;
  uint64_t vttbr;
  char *hyp_msg;
  vcpu_reg_t reg;
  vcpu_sysreg_t sysreg;
  hyp_security_t security;
  struct{
    uint32_t vserror_pending;
    uint32_t virq_pending;
    uint32_t vfiq_pending;
    uint32_t core_timer_intr_enable;
    uint32_t core_mbox_intr_enable;
    uint32_t core_mbox[4];
  }vic;
} vcpu_t;

vcpu_t *vcpu_create(vm_t *vm, uint32_t vcpu_id,
                phys_addr_t vttbr, char *hyp_msg, phys_addr_t entry_addr);
void vcpu_ready(vcpu_t *vcpu);
void vcpu_active(vcpu_t *vcpu, pcpu_t *phys_cpu);
void vcpu_sleep(vcpu_t *vcpu);
void vcpu_off(vcpu_t *vcpu);
void vcpu_do_vserror(vcpu_t *vcpu);
void vcpu_do_virq(vcpu_t *vcpu);
void vcpu_do_vfiq(vcpu_t *vcpu);
void vcpu_save_all_sysregs(vcpu_t *vcpu);
void vcpu_restore_all_sysregs(vcpu_t *vcpu);
void vcpu_context_save(vcpu_t *vcpu, vcpu_reg_t *vcpu_reg);
void vcpu_context_switch(vcpu_t *vcpu);
vcpu_t *set_cur_vcpu(uint64_t cpu_id, vcpu_t *vcpu);
vcpu_t *get_cur_vcpu(uint64_t cpu_id);

#include "coproc_def.h"
static inline int vcpu_currentel_get(vcpu_t *vcpu){
  return (vcpu->sysreg.cpsr&CPSR_M_EL1t)? 1:0;
}


#endif
