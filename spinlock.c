#include "typedef.h"
#include "lib.h"
#include "asm_func.h"
#include "pcpu.h"
#include "spinlock.h"

void spin_lock(int *locked){
  pcpu_t *cpu = get_current_phys_cpu();

  /* Save interrupt enable */
  READ_SYSREG(cpu->intr_state, DAIF);
  INTR_DISABLE;

  asm volatile(
    "mov x1, %0\n"
    "mov w2, #1\n"
    "prfm pstl1keep, [x1]\n"
    "1: ldaxr w3, [x1]\n"
    "cbnz w3, 1b\n"
    "stxr w3, w2, [x1]\n"
    "cbnz w3, 1b\n"

    : : "r" ((phys_addr_t)locked)
  );
}

void spin_unlock(int *locked){  
  pcpu_t *cpu = get_current_phys_cpu();

  asm volatile(
    "mov x1, %0\n"
    "stlr wzr, [x1]\n"

    : : "r" ((phys_addr_t)locked)
  );
  WRITE_SYSREG(DAIF, cpu->intr_state);
}
