/*
 * arm_timer.c
 *    Driver for ARM Generic timers
 */
#include "typedef.h"
#include "asm_func.h"
#include "arm_timer.h"

void arm_timer_init(generic_timer_t tmr_kind){
  switch(tmr_kind){
    case CNTV_EL0:
      WRITE_SYSREG(CNTV_CTL_EL0, 0);
      break;
    case CNTP_EL0:
      WRITE_SYSREG(CNTP_CTL_EL0, 0);
      break;
    case CNTPS_EL1:
      WRITE_SYSREG(CNTPS_CTL_EL1, 0);
      break;
    case CNTHP_EL2:
      WRITE_SYSREG(CNTHP_CTL_EL2, 0);
      break;
  }
}

void arm_timer_start(generic_timer_t tmr_kind){
  switch(tmr_kind){
    case CNTV_EL0:
      WRITE_SYSREG(CNTV_CTL_EL0, CNTxx_CTL_ENABLE);
      break;
    case CNTP_EL0:
      WRITE_SYSREG(CNTP_CTL_EL0, CNTxx_CTL_ENABLE);
      break;
    case CNTPS_EL1:
      WRITE_SYSREG(CNTPS_CTL_EL1, CNTxx_CTL_ENABLE);
      break;
    case CNTHP_EL2:
      WRITE_SYSREG(CNTHP_CTL_EL2, CNTxx_CTL_ENABLE);
      break;
  }
}

int arm_timer_is_pending(generic_timer_t tmr_kind){
  uint64_t cntxx_ctl;
  switch(tmr_kind){
    case CNTV_EL0:
      READ_SYSREG(cntxx_ctl, CNTV_CTL_EL0);
      break;
    case CNTP_EL0:
      READ_SYSREG(cntxx_ctl, CNTP_CTL_EL0);
      break;
    case CNTPS_EL1:
      READ_SYSREG(cntxx_ctl, CNTPS_CTL_EL1);
      break;
    case CNTHP_EL2:
      READ_SYSREG(cntxx_ctl, CNTHP_CTL_EL2);
      break;
  }
  return (cntxx_ctl&CNTxx_CTL_ISTATUS)? 1:0;
}

int arm_timer_pending_clear(generic_timer_t tmr_kind){
  uint64_t cntxx_ctl;
  switch(tmr_kind){
    case CNTV_EL0:
      READ_SYSREG(cntxx_ctl, CNTV_CTL_EL0);
      cntxx_ctl = ~(uint64_t)CNTxx_CTL_ISTATUS;
      break;
    case CNTP_EL0:
      READ_SYSREG(cntxx_ctl, CNTP_CTL_EL0);
      break;
    case CNTPS_EL1:
      READ_SYSREG(cntxx_ctl, CNTPS_CTL_EL1);
      break;
    case CNTHP_EL2:
      READ_SYSREG(cntxx_ctl, CNTHP_CTL_EL2);
      break;
  }
  return (cntxx_ctl&CNTxx_CTL_ISTATUS)? 1:0;
}
