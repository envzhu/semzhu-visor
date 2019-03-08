#ifndef _HYP_SECURITY_H_INCLUDED_
#define _HYP_SECURITY_H_INCLUDED_

typedef struct _hyp_security_t hyp_security_t;

#include "typedef.h"
#include "vcpu.h"

#define HYP_SEC_RET_ERROR 1
#define HYP_OLD_SEC_RET   100
#define HYP_SEC_RET   0x1000
#define HYP_SEC_CALL  0x1100
#define HYP_SEC_JMP   102

typedef struct _hyp_security_t {
  int error;
  uint64_t sec_ret_counter;
  uint64_t sec_blr_counter;
} hyp_security_t;

void hyp_security_vcpu_init(vcpu_t *vcpu);
/* Hypervisor security function */
int hyp_security_check(vcpu_t *vcpu, uint32_t esr);
void hyp_security_error(vcpu_t *vcpu);

#endif
