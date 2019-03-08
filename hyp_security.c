#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "coproc_def.h"
#include "vcpu.h"
#include "hyp_security.h"


static *sec_error_msg[] = {
  "There is no security error.",
  "A ROP attack is detected.",
  "A COP attack is detected.",
};

void hyp_security_vcpu_init(vcpu_t *vcpu){
  vcpu->security.error = 0;
  vcpu->security.sec_ret_counter = 0;
  vcpu->security.sec_blr_counter = 0;
}

int hyp_security_check(vcpu_t *vcpu, uint32_t esr){ 
  uint8_t ec = esr >> 26;
  uint16_t iss = esr & 0x1FFF;
  uint32_t caller_code;
  uint32_t callee_code;
  uint16_t hvc_type;

  if(ec == EC_UNKNOWN_REASON){
    hvc_type = extract(*(uint32_t *) el1va2pa(vcpu->sysreg.pc), 5, 20);
    vcpu->sysreg.pc += 4;
  }
  else if(ec == EC_HVC_A64)   
    hvc_type = iss;
  else
    return -1;
  
  switch(hvc_type&0xFFE0){
    case HYP_SEC_RET:
      //vcpu->security.sec_ret_counter++;

      caller_code = *(uint32_t *) el1va2pa(vcpu->reg.x[30]-4);
      
      /* call-precededness check */

      /* If this caller instruction is bl or blr, this is not rop attack. */
      if(((caller_code>>26)==0b100101)
          || ((caller_code&0xFFFFFC1F)==0xD63F0000)){
        vcpu->sysreg.pc = vcpu->reg.x[30];
        // log_debug("[Hyp security] Caller check : ok!\n");

      /* If this caller instruction is neither bl nor blr, this is ROP attack. */  
      }else{
        log_error("caller ipa : %#8x, caller code : %#8x\n", el1va2ipa(vcpu->reg.x[30]-4), caller_code);
        log_error("This is not a call instruction.\nSo it is ROP!!!!!\n");
        vcpu->security.error = 1;
        return -1;
      }
      WRITE_SYSREG(ELR_EL2, vcpu->sysreg.pc);
      dispatch(vcpu);
      break;

    case HYP_SEC_CALL:
      vcpu->security.sec_blr_counter++;
      callee_code = *(uint32_t *) el1va2pa(vcpu->reg.x[hvc_type&0x1F]-4);
      
      /* emulate blr instruction */
      vcpu->reg.x[30] = vcpu->sysreg.pc;
      vcpu->sysreg.pc = vcpu->reg.x[hvc_type&0x1F];
      WRITE_SYSREG(ELR_EL2, vcpu->sysreg.pc);
      /*log_debug("New $PC : %#8x, $LR : %#8x\n",
          vcpu->sysreg.pc, vcpu->reg.x[30]);*/

      /* return-precededness check */
      /*
      if(callee_code != 0xd65f03c0){
        log_error("caller ipa : %#8x, caller code : %#8x\ncallee ipa : %#8x, callee code : %#8x\n",
            el1va2ipa(vcpu->sysreg.pc-4), *(uint32_t *)el1va2pa(vcpu->sysreg.pc-4), el1va2ipa(vcpu->reg.x[hvc_type&0x1F]-4), callee_code);
        log_error("COP!!!!!\n");
        vcpu->security.error = 2;
        return -1;
      }*/

      break;
    default:
      /* This interrupt is not a security interrupt. */
        READ_SYSREG(vcpu->sysreg.pc, ELR_EL2);

      return -1;
  }
  WRITE_SYSREG(ELR_EL2, vcpu->sysreg.pc);
  dispatch(vcpu);
  return 1;
}

void hyp_security_error(vcpu_t *vcpu){
  int vcpu_currentel = vcpu_currentel_get(vcpu);
  log_error("%s in in EL%d\n",
      sec_error_msg[vcpu->security.error], vcpu_currentel);
  log_error("  vm_name : %s vcpu_id : %d \n",
      vcpu->vm->name, vcpu->vcpu_id);
  vcpu->security.error = 0;
  
  if(vcpu_currentel == 0)
    vcpu_do_vserror(vcpu);
  else
    vm_force_shutdown(vcpu->vm);
}
