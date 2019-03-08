#ifndef _HYP_CALL_H_INCLUDED_
#define _HYP_CALL_H_INCLUDED_

 #include "typedef.h"

#define HYP_VM_MSG_SIZE  0x1000

/* Hypervisor Call */
void hyp_call(vcpu_t *vcpu, uint64_t type);

#endif
