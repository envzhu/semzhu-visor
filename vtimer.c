
/* 
 * vtimer.c
 * Control EL0 virtual timer
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "hyp_timer.h"
#include "vcpu.h"

/*
 * Generic Virtual Timer registers for Hypervisor
 * - CNTFRQ_EL0     32-bit Counter-timer Frequency register
 * - CNTVCT_EL0     64-bit Counter-timer Virtual Count register
 * - CNTV_TVAL_EL0  32-bit Counter-timer Virtual Timer TimerValue register
 * - CNTV_CTL_EL0   32-bit Counter-timer Virtual Timer Control register
 * - CNTV_CVAL_EL0  64-bit Counter-timer Virtual Timer CompareValue register
 * - CNTVOFF_EL2    64-bit Counter-timer Virtual Offset register
 * - CNTHCTL_EL2    32-bit Counter-timer Hypervisor Control register
 */

/*
 * CNTHCTL_EL2 bits
 * When HCR_EL2.E2H == 0
 */

#define CNTHCTL_EL1PCTEN  (1<<0)
  /* 
   * If 0,
   * Non-secure EL0 and EL1 accesses to the CNTPCT_EL0 (physical counter register)
   * are trapped to EL2, unless it is trapped by CNTKCTL_EL1.EL0PCTEN.
  */

#define CNTHCTL_EL1PCEN (1<<1)
/*
 * If 0,
 * Non-secure EL0 and EL1 accesses 
 * to the physical counter register(CNTP_CTL_EL0, 
 * CNTP_CVAL_EL0, CNTP_TVAL_EL0) are trapped to EL2,
 * unless it is trapped by CNTKCTL_EL1.EL0PTEN.
 */

#define CNTHCTL_EVNTEN  (1<<2)
#define CNTHCTL_EVNTDIR (1<<3)
#define CNTHCTL_EVNTI(x) (x<<4)

void vtimer_init(void){
  //vtimer_reg_reset();
}

void vtimer_reg_reset(void){
  WRITE_SYSREG(CNTV_CVAL_EL0, 0);
}

static void emulate_vtimer_handler(pcpu_t *phys_cpu, vcpu_t *vcpu){
  log_info("emulate_vtimer_handler()\n");
  vcpu->sysreg.cntv_ctl_el0 |= CNTxx_CTL_ISTATUS;
  vcpu_do_virq(vcpu);
}

void emulate_vtimer(vcpu_t *vcpu){
  uint64_t cntv_ctl_el0;
  uint64_t cntv_tval_el0;
  uint64_t cntv_cval_el0;
  uint64_t cntvct_el0;

  READ_SYSREG(cntv_ctl_el0, cntv_ctl_el0);
  READ_SYSREG(cntv_tval_el0, cntv_tval_el0);
  READ_SYSREG(cntv_cval_el0, cntv_cval_el0);
  READ_SYSREG(cntvct_el0, cntvct_el0);

  log_info("cntv_ctl_el0 : %d, cntv_tval_el0 : %d, cntv_cval_el0 : %d, cntv_off : %d\n",
      cntv_ctl_el0, cntv_tval_el0, cntv_cval_el0, vcpu->sysreg.cntvoff_el2);

  if(vcpu->phys_cpu->scheduler->emulate_vtimer == 1){
    if(vcpu->sysreg.cntv_ctl_el0&CNTxx_CTL_ENABLE){
      timer_event_add(get_current_phys_cpu(), emulate_vtimer_handler,
          hyp_timer_tick2msec(cntv_cval_el0 - cntvct_el0), vcpu);
      log_debug("emule vtimer() %dticks %dmsec\n",
        cntv_tval_el0 - cntvct_el0, hyp_timer_tick2msec(cntv_cval_el0 - cntvct_el0));
    }
  }
}

void vtimer_context_save(vcpu_t *vcpu){
  /*TODO*/
  asm volatile("isb");
  // READ_SYSREG(vcpu->tvoff, CNTVOFF_EL2);
}

void vtimer_context_restore(vcpu_t *vcpu){
  // WRITE_SYSREG(CNTVOFF_EL2, vcpu->tvoff);
  asm volatile("isb");
}
