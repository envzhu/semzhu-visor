#include "coproc_def.h"

.section .text.boot , "ax"
.global startup_entry

// $x0 = Physical address to the FDT blob.
startup_entry:
  mrs	  x1, CurrentEL

  /* If CurrentEL is EL3, start as EL3 */
  cmp   x1, #0xc
  beq   init_smc

  /* If CurrentEL is EL2, start as EL2 */
  cmp   x1, #0x8
  beq   init_hyp
  
  /*
   * If CurrentEL is EL1 or EL0,
   * we cannnot start hypervisor, so stop 
   */
  b     cpu_hlt

/* Initialize EL3 (Secure Monitor mode) */
init_smc:
  // set scr_el3
  mov   x1, #1
  orr   x1, x1, #SCR_NS
  // orr   x1, x1, #SCR_SMD
  orr   x1, x1, #SCR_HCE
  orr   x1, x1, #SCR_RW
  msr   scr_el3, x1

  // set spsr_el3
  mov   x1, #0xc9    // D=1, A=1, I=1, F=1 M=EL2h
  msr   spsr_el3, x1

  adr   x1, init_hyp
  msr   elr_el3, x1
  eret

/* Initialize EL2 (hypervisor mode) */
init_hyp:
  // read cpu id, stop slave cores
  mrs     x1, mpidr_el1
  and     x1, x1, #3
  
  /* Set stack of each cpu */
  /* $sp = _stack_end - _stack_size * cpuid  */
  ldr   x2, = _stack_end
  ldr	  x3, = _stack_size
  mul   x3, x1, x3

  sub   x2, x2, x3
  mov   sp, x2

  // set vector address
  ldr   x2, = el2_vector
  msr   vbar_el2, x2
  
  // If cpu id != 0, wait for primary cpu to be ready. 
  cbnz  x1, start_secondary
  
  // clear BSS
  // x1: BSS start address
  // x2: BSS end address
  ldr   x1, =_bss_start
  ldr   x2, =_bss_end
clear_bss:
  cmp   x1, x2
  beq   clear_done
  str   xzr, [x1]
  add   x1, x1, #4
  b clear_bss
clear_done:
  // $x0 = Physical address to the FDT blob.
  b   start_primary
  