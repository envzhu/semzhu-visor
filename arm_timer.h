#ifndef _ARM_TIMER_H_INCLUDED_
#define _ARM_TIMER_H_INCLUDED_

#include "typedef.h"
#include "coproc_def.h"

typedef enum{
  CNTV_EL0 = 0,
  CNTP_EL0,
  CNTPS_EL1,
  CNTHP_EL2,
}generic_timer_t

#endif
