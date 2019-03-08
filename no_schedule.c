/*
 * no_scheduler.c :
 *   do not schedule, 
 *   only assign one guest VM per one physical cpu core
 */


#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "vm.h"
#include "smp_mbox.h"
#include "vcpu.h"
#include "schedule.h"

static scheduler_init_fn_t    no_scheduler_init;
static scheduler_add_fn_t     no_scheduler_add;
static scheduler_remove_fn_t  no_scheduler_remove;
static schedule_fn_t          no_schedule;
static scheduler_dump_ready_vcpu_fn_t  no_schedule_dump_ready_vcpu;

scheduler_t no_scheduler = {
  {NULL, NULL, NULL, NULL},
  0,
  0,
  0,
  no_scheduler_init,
  no_scheduler_add,
  no_scheduler_remove,
  no_schedule,
  no_schedule_dump_ready_vcpu,
};

/*
 * no_scheduler_init() is called in scheduler_init()
 * after no_scheduler.phys_cpu and no_scheduler.pcpu?num are set.
 */
static void no_scheduler_init(void){
  int i;

}

/* Add vcpu to the tail of ready que */
static void no_scheduler_add(vcpu_t *vcpu){
  int i;
  vcpu_t *t_vcpu;

  if(vcpu == NULL)
    hyp_panic("You cannnot add NULL vcpu to readyque.\n");

  /* Set schedule need flag of each cpu which uses this scheduler */
  for(i=0; i<no_scheduler.pcpu_num; i++){
    if(no_scheduler.phys_cpu[i]->current_vcpu == NULL){
      no_scheduler.phys_cpu[i]->current_vcpu = vcpu;
      break;
    }
  }
  if(i == no_scheduler.pcpu_num)
    hyp_panic("There is no free physical cpu to run this vcpu.\n");
}

static void no_scheduler_remove(vcpu_t *vcpu){

  hyp_panic("You cannnot remove a vcpu from no readyque.\n");
}

static void no_schedule(pcpu_t *phys_cpu){
  int i;
  
}

static void no_schedule_dump_ready_vcpu(log_level_t level){
  int i;
  vcpu_t *t_vcpu;

  log_printf(level, "Start dump vcpu in no scheduler ready que\n");

  /* Set schedule need flag of each cpu which uses this scheduler */
  for(i=0; i<no_scheduler.pcpu_num; i++){
    if(no_scheduler.phys_cpu[i]->current_vcpu != NULL){
      t_vcpu = no_scheduler.phys_cpu[i]->current_vcpu;
      log_printf(level, "ready vm:%s vcpu_id:%d\n",
          t_vcpu->vm->name, t_vcpu->vcpu_id);
    }
  }

  log_printf(level, "=================   End   =================\n");
}
