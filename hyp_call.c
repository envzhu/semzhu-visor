#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "coproc_def.h"
#include "pmu.h"
#include "vcpu.h"
#include "vm.h"
#include "hyp_call.h"
#include "hyp_mmu.h"

#define HYP_CALL_PUTS       0
#define HYP_CALL_FORCE_SHUTDOWN 1
#define HYP_CALL_SEND_MAIL  2
#define HYP_CALL_RECV_MAIL  3
#define HYP_CALL_CYCLE_COUNT_START 4
#define HYP_CALL_CYCLE_COUNT_READ  5
#define HYP_CALL_CYCLE_COUNT_STOP  6

void hyp_call(vcpu_t *vcpu, uint64_t type){

  log_debug("HVC #%#x\n", type);

  switch(type){
    case HYP_CALL_PUTS:
      //log_debug("%#x\n", vcpu->hyp_msg);
      log_info("[from vm %s] %s", vcpu->vm->name, vcpu->hyp_msg);
      break;

    case HYP_CALL_FORCE_SHUTDOWN:
      log_info("Force Shutdown this VM\n");
      vm_force_shutdown(vcpu->vm);
      break;

    case HYP_CALL_CYCLE_COUNT_START: 
      log_info("CPU execute cycle counter start\n");
      vcpu->security.sec_ret_counter = 0;
      vcpu->security.sec_blr_counter = 0;
      cycle_count_start();
      break;

    case HYP_CALL_CYCLE_COUNT_READ:
      log_info("CPU execute cycle counter value : %#x\n", cycle_count_stop());
      log_info("sec_ret_counter value : %#x\n", vcpu->security.sec_ret_counter);
      log_info("sec_blr_counter value : %#x\n", vcpu->security.sec_blr_counter);
      break;
      
    case HYP_CALL_CYCLE_COUNT_STOP: 
      log_info("CPU execute cycle count : %#x\n", cycle_count_stop());
      break;
    
    default:
      log_error("Illegal Hypervisor call : HVC #%#x\n", type);
      vcpu_do_vserror(vcpu);
  }
}
