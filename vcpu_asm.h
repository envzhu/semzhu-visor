#ifndef _VCPU_ASM_H_INCLUDED_
#define _VCPU_ASM_H_INCLUDED_

#define VCPU_OFF_REG_X(m) (64+ m*8)
#define VCPU_OFF_REG_Q(m) (320 + m*16)

#define VCPU_OFF_REG_LR VCPU_OFF_REG_X(30)
#define VCPU_OFF_REG_PC 832

#define VCPU_OFF_SEC_HEAD(x) (984+x)
#define VCPU_OFF_SEC_ERROR VCPU_OFF_SEC_HEAD(0)
#define VCPU_OFF_SEC_RET_CNT VCPU_OFF_SEC_HEAD(8)
#define VCPU_OFF_SEC_BLR_CNT VCPU_OFF_SEC_HEAD(16)

#endif