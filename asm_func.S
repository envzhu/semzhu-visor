#include "vcpu_asm.h"

.section .text , "ax"

  .global cpu_relax
cpu_relax:
  /* Make this cpu sleep until an exception is caused. */
  
  msr  daifclr, #0b1111   /* Enable Interrupt */
  wfe
  
  /* 
   * This cpu will be never come back here 
   * because on Senju-visor cpu does not return from exception from EL2.
   */
  b   cpu_relax /* For qemu, qemu emulate wfe as nop */

  .global cpu_hlt
cpu_hlt:
  /* Make this cpu sleep forever. */
  msr  daifset, #0b1111   /* Disable Interrupt */
  wfe
  /* This cpu will be never waken up. */
  b   cpu_hlt

  .global dispatch
dispatch:
  /* $x0 must indicate &vcpu */

  /* $sp = &vcpu->reg.x[0] */
  add   sp, x0, #VCPU_OFF_REG_X(0)
  
  /* restore general purpose registers */
  ldp   x0,  x1,  [sp], #16
  ldp   x2,  x3,  [sp], #16
  ldp   x4,  x5,  [sp], #16
  ldp   x6,  x7,  [sp], #16
  ldp   x8,  x9,  [sp], #16
  ldp   x10, x11, [sp], #16
  ldp   x12, x13, [sp], #16
  ldp   x14, x15, [sp], #16
  ldp   x16, x17, [sp], #16
  ldp   x18, x19, [sp], #16
  ldp   x20, x21, [sp], #16
  ldp   x22, x23, [sp], #16
  ldp   x24, x25, [sp], #16
  ldp   x26, x27, [sp], #16
  ldp   x28, x29, [sp], #16
  ldp   x30, xzr, [sp], #16
  eret

  .global vcpu_freg_save
vcpu_freg_save:
  /* $x0 must indicate &vcpu */

  /* $x1 = &vcpu->reg.q[0] */
  add   x1, x0, #VCPU_OFF_REG_Q(0)

  /* save floating-point registers */
  
  stp   q0,  q1,  [x1], #32
  stp   q2,  q3,  [x1], #32
  stp   q4,  q5,  [x1], #32
  stp   q6,  q7,  [x1], #32
  stp   q8,  q9,  [x1], #32
  stp   q10, q11, [x1], #32
  stp   q12, q13, [x1], #32
  stp   q14, q15, [x1], #32
  stp   q16, q17, [x1], #32
  stp   q18, q19, [x1], #32
  stp   q20, q21, [x1], #32
  stp   q22, q23, [x1], #32
  stp   q24, q25, [x1], #32
  stp   q26, q27, [x1], #32
  stp   q28, q29, [x1], #32
  stp   q30, q31, [x1], #32

  ret

  .global vcpu_freg_restore
vcpu_freg_restore:
  /* $x0 must indicate &vcpu */

  /* $x0 = &vcpu->reg.q[0] */
  add   x1, x0, #VCPU_OFF_REG_Q(0)
  
  /* restore floating-point registers */
  ldp   q0,  q1,  [x1], #32
  ldp   q2,  q3,  [x1], #32
  ldp   q4,  q5,  [x1], #32
  ldp   q6,  q7,  [x1], #32
  ldp   q8,  q9,  [x1], #32
  ldp   q10, q11, [x1], #32
  ldp   q12, q13, [x1], #32
  ldp   q14, q15, [x1], #32
  ldp   q16, q17, [x1], #32
  ldp   q18, q19, [x1], #32
  ldp   q20, q21, [x1], #32
  ldp   q22, q23, [x1], #32
  ldp   q24, q25, [x1], #32
  ldp   q26, q27, [x1], #32
  ldp   q28, q29, [x1], #32
  ldp   q30, q31, [x1], #32
  
  ret
