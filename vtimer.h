#ifndef _VTIMER_H_INCLUDED_
#define _VTIMER_H_INCLUDED_

#include "typedef.h"
#include "pcpu.h"
#include "vm.h"
// ISB
void vtimer_init(void);
void vtimer_reg_reset(void);
void emulate_vtimer(vcpu_t *vcpu);
void vtimer_context_save(vcpu_t *vcpu);
void vtimer_context_restore(vcpu_t *vcpu);


#endif
