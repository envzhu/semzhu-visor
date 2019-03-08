#include "vcpu_asm.h"
#include "coproc_def.h"

.section .text , "ax"

/*
 * $x0 = &current_vcpu.reg.x[0]
 * $x1 = esr(Exception syndrome Register)
 * $x2 = esr >>26 ;ec(Exception Class)
 * $x3= security_type
 * $x4= ELR_EL2 ;current_vcpu.pc
 * $x5= pa of ELR_EL2 ; pa of current_vcpu.pc
 * $x6= opcode
 * $x7 : temp
 * $x8 : temp
 * $x9 : temp
 */
.global hyp_security_check_fast
hyp_security_check_fast:

  /* $x2 = (uint32_t) esr >> 26; */
  lsr	w2, w1, #26

  /* If EC == EC_UNKNOWN_REASON */
  cbz	w2, get_security_type_from_pc
	
  /* If EC == EC_HVC_A64 */
  cmp	w2, #EC_HVC_A64
	b.eq	get_security_type_from_iss

  b end_not_security

get_security_type_from_iss:
  /* $x4 = current_vcpu->sysreg.pc */
  ldr	x4, [x0, #VCPU_OFF_REG_PC]
  sub x4, x4, #4

  /* security_type = iss = esr & 0x1fff */
  and	w3, w1, #0x1fff
  
  b check_security_type

get_security_type_from_pc:
  /* $x4 = current_vcpu->sysreg.pc */
  ldr	x4, [x0, #VCPU_OFF_REG_PC]

  at	s12e1r, x4
  mrs	x5, par_el1
  and	x5, x5, #0xfffff000
  
  and x8, x4, #0xfff
  add x5, x5, x8
  
  ldr	w6, [x5, #0]

  lsr	w3, w6, #5
  and w3, w3, #0xffff
  
  /* b  check_security_type */

check_security_type:

  cmp	w3, #100
	b.eq	sec_ret

  and w8, w3, #0xffe0
  cmp	w8, #0x1000
	b.eq	sec_ret


  mov w7, #0x1100
  cmp	w8, w7
	b.eq  sec_blr

  b end_not_security
sec_ret:
  ldr x8, [x0, #VCPU_OFF_SEC_RET_CNT]
  add x8, x8, 1
  str x8, [x0, #VCPU_OFF_SEC_RET_CNT]
    
  /* $x7 = current_vcpu.reg.x[30] */
  ldr	x7, [x0, #VCPU_OFF_REG_LR]
  subs	x7, x7, #4
  at	s12e1r, x7
  mrs	x8, par_el1
  and	x8, x8, #0xfffff000
  
  and x7, x7, #0xfff
  add x7, x7, x8


  ldr	w6, [x7, #0]

  lsr	w8, w6, #26

  cmp	w8, #0b100101
	b.eq	sec_ret_correctly_end
  
  and	  w8, w6, #0xFFFFFC1F
  mov   w7, #0xD63F0000
  cmp	  w8, w7
	b.eq	sec_ret_correctly_end
  
sec_ret_illegal_end: 
  mov   x7, #1
  str   x7, [x0, #984]
  ret

sec_ret_correctly_end:
  /* $PC = return address */
  ldr	  x5, [x0, #VCPU_OFF_REG_LR]
  str	  x5, [x0, #VCPU_OFF_REG_PC]

  b     end_security

/* Mitigation for COP */ 
sec_blr:
  /* Increment vcp.security.sec_blr_counter */

  ldr   x8, [x0, #VCPU_OFF_SEC_BLR_CNT]
  add   x8, x8, #1
  str   x8, [x0, #VCPU_OFF_SEC_BLR_CNT]
  
  /* $x8 = Rn */
  and   x8, x3, #0x1f

  /* $x4 = current_vcpu.reg.x[Rn] */
  /*  $x2 = #VCPU_OFF_REG_X + $x8*8 */
  mov   x2, #VCPU_OFF_REG_X(0)
  mov   x9, #8
  madd  x2, x8, x9, x2
  ldr	  x4, [x0, x2]

  /* $x7 = PA of current_vcpu.reg.x[Rn] */

  at	  s12e1r, x4
  mrs	  x8, par_el1
  and	  x8, x8, #0xfffff000
  
  and   x7, x4, #0xfff
  add   x7, x7, x8
  
  /* return-precededness check */
  
  /* $x6 = opcode of current_vcpu.reg.x[Rn] - 4 */
  ldr	w6, [x7, #-4]

  and	  w8, w6, #0xFFFFFC1F
  mov   w9, #0xd65f0000
  cmp	  w8, w9
	b.eq	sec_blr_correctly_end

  #if 1
  /* function magic check */
  
  /* $x6 = opcode of current_vcpu.reg.x[Rn] + 0 */
  ldr	w6, [x7, #0]

  and	  w8, w6, #0xFFC07FFF
  mov   w9, #0x7BFD
  movk  w9, #0xA980, lsl #16
  cmp	  w8, w9
	b.eq	sec_blr_correctly_end
  #endif

sec_blr_illegal_end:
  mov   x7, #2
  str   x7, [x0, #VCPU_OFF_SEC_ERROR]
  ret

/* emulate normal blr instruction */
sec_blr_correctly_end:
  /* $LR = $PC + 4 */
  ldr	x5, [x0, #VCPU_OFF_REG_PC]
  add x5, x5, #4
  str	x5, [x0, #VCPU_OFF_REG_LR]

  str x4, [x0, #VCPU_OFF_REG_PC]

end_security:
  add   sp, x0, #VCPU_OFF_REG_X(0)
  
  ldr x1, [x0, #VCPU_OFF_REG_PC]
  msr	ELR_EL2, x1
  
  /* restore general purpose registers */
  ldp   x0,  x1,  [sp], #16
  ldp   x2,  x3,  [sp], #16
  ldp   x4,  x5,  [sp], #16
  ldp   x6,  x7,  [sp], #16
  ldp   x8,  x9,  [sp], #16
  /*
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
  */
  add   sp, sp, #160 /* 160 = 10 *16 */
  ldp   x30, xzr, [sp], #16
  eret

end_not_security:
  ret
