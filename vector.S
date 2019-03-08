#include "vcpu_asm.h"

.section .vector , "ax"

.macro hyp_ventry handler vec_num
  /*
   * When an exception was taken in hyp mode,
   * we do not save registers
   * because we never restore to hyp mode from exception handler
   */
  // read cpu id
  mrs     x1, mpidr_el1
  and     x1, x1, #3
  
  /* Set stack of each cpu */
  /* $sp = _stack_end - _stack_size * cpuid  */
  ldr   x2, = _stack_end
  ldr	  x3, = _stack_size
  mul   x3, x1, x3

  sub   x2, x2, x3
  mov   sp, x2
  
  mov   x0, \vec_num
  mrs   x1, esr_el2

  b     \handler
.endmacro

.macro vm_entry_general_reg_save
  /*
   * We save only general purpose registers in the interrupt vector
   * due to the small vecor space.
   * So we need to save other registers if we switch to another guest OS.
   */
  /* sp_el2 must indicate &current_vcpu->reg.x[32] */
  /* save general purpose registers to current_vcpu->reg.x */
  stp   x30, xzr, [sp, #-16]!
  stp   x28, x29, [sp, #-16]!
  stp   x26, x27, [sp, #-16]!
  stp   x24, x25, [sp, #-16]!
  stp   x22, x23, [sp, #-16]!
  stp   x20, x21, [sp, #-16]!
  stp   x18, x19, [sp, #-16]!
  stp   x16, x17, [sp, #-16]!
  stp   x14, x15, [sp, #-16]!
  stp   x12, x13, [sp, #-16]!
  stp   x10, x11, [sp, #-16]!
  stp   x8,  x9,  [sp, #-16]!
  stp   x6,  x7,  [sp, #-16]!
  stp   x4,  x5,  [sp, #-16]!
  stp   x2,  x3,  [sp, #-16]!
  stp   x0,  x1,  [sp, #-16]!
  
.endmacro

.macro vm_entry_stackpointer_set
  // read cpu id
  mrs     x1, mpidr_el1
  and     x1, x1, #3
  
  /* Set stack of each cpu */
  /* $sp = _stack_end - _stack_size * cpuid  */
  ldr   x2, = _stack_end
  ldr	  x3, = _stack_size
  mul   x3, x1, x3

  mov   x4, sp
  sub   x2, x2, x3
  mov   sp, x2

.endmacro

.macro vm_ventry handler vec_num
  
  vm_entry_general_reg_save

  vm_entry_stackpointer_set

  mov   x0, \vec_num
  mrs   x1, esr_el2

  b     \handler
.endmacro

.global el2_vector
// vector table
el2_vector:
.balign 0x800
  // Synchronous  Current EL with SP0
  hyp_ventry hyp_error_interrupt_handler 0

.balign 0x80
  // IRQ/vIRQ  Current EL with SP0
  hyp_ventry hyp_error_interrupt_handler 1

.balign 0x80
  // FIQ/vFRQ  Current EL with SP0
  hyp_ventry hyp_error_interrupt_handler 2

.balign 0x80
  // SError/vSError  Current EL with SP0
  hyp_ventry hyp_error_interrupt_handler 3

.balign 0x80
  // Synchronous  Current EL with SPx
  hyp_ventry hyp_error_interrupt_handler 4

.balign 0x80
  // IRQ/vIRQ  Current EL with SPx
  hyp_ventry hyp_irq_interrupt_handler 5

.balign 0x80
  // FIQ/vFRQ  Current EL with SPx
  hyp_ventry vm_interrupt_entry 6

/* TODO */
.balign 0x80
  // SError/vSError  Current EL with SPx
  hyp_ventry hyp_error_interrupt_handler 7
  
.balign 0x80
  // Synchronous  Lower EL using AArch64
  
  /* Save general purpose registers */
  vm_entry_general_reg_save

  sub	x0, sp, #VCPU_OFF_REG_X(0)
  
  mrs	x1, ELR_EL2
  str x1, [x0, #VCPU_OFF_REG_PC]
  
  mrs   x1, esr_el2

  bl    hyp_security_check_fast
  
  vm_entry_stackpointer_set

  mov   x0, #8
  mrs   x1, esr_el2

  b     vm_interrupt_entry
  
  .balign 0x80
  // IRQ/vIRQ  Lower EL using AArch64
  vm_ventry vm_irq_interrupt_entry 9

.balign 0x80
  // FIQ/vFRQ  Lower EL using AArch64
  vm_ventry vm_interrupt_entry 10

.balign 0x80
  // SError/vSError  Lower EL using AArch64
  vm_ventry vm_interrupt_entry 11

.balign 0x80
  // Synchronous  Lower EL using AArch32
  vm_ventry aarch32_interrupt_handler 12

.balign 0x80
  // IRQ/vIRQ  Lower EL using AArch32
  vm_ventry aarch32_interrupt_handler 13

.balign 0x80
  // FIQ/vFRQ  Lower EL using AArch32
  vm_ventry aarch32_interrupt_handler 14

.balign 0x80
  // SError/vSError  Lower EL using AArch32
  vm_ventry aarch32_interrupt_handler 15
