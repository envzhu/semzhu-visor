#ifndef _ASM_FUNC_H_INCLUDED_
#define _ASM_FUNC_H_INCLUDED_

#include "typedef.h"
#include "vcpu.h"

// Pototype declarations of Assembly function in asm_func.S
void cpu_relax(void);   /* Stop CPU until an exception is caused */
void cpu_hlt(void);   /* Stop CPU forever */
void dispatch(vcpu_t *vcpu);  /* dispatcher for hypervisor */
void vcpu_freg_save(vcpu_t *vcpu);
void vcpu_freg_restore(vcpu_t *vcpu);


// Assembly defines for C
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)
#define READ_SYSREG(dst, sys_reg) ({ \
	asm volatile("mrs %0, " __stringify(sys_reg) : "=r" (dst));\
})
#define WRITE_SYSREG(sys_reg, v) do { \
	uint64_t __val = (uint64_t)(v); \
	asm volatile("msr " __stringify(sys_reg) ", %x0"\
		     : : "rZ" (__val)); \
} while (0)

#define READ_CPUID(x) do{ \
  READ_SYSREG(x, MPIDR_EL1); \ 
  x &= 0xFF;  \
}while (0)

#define INTR_ENABLE   asm volatile("msr  daifclr, #0b1111")
#define INTR_DISABLE  asm volatile("msr  daifset, #0b1111")

/*
 * NOTE:  TODO
 * 
 */
static inline void get_args(uint64_t *argv){
  asm volatile(
    "mov    %0, x0\n\t"
    "mov    %1, x1\n\t"
    "mov    %2, x2\n\t"
    "mov    %3, x3\n\t"
    "mov    %4, x4\n\t"
    "mov    %5, x5\n\t"
    "mov    %6, x6\n\t"
    "mov    %7, x7\n\t"
    : "=r" (argv[0]), "=r" (argv[1]), "=r" (argv[2]), "=r" (argv[3]), "=r" (argv[4]), "=r" (argv[5]), "=r" (argv[6]), "=r" (argv[7])
  );
}

#endif
