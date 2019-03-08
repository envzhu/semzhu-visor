/*
 * fcfs_scheduler.c :
 *   Fixed-priority pre-emptive scheduler
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

static scheduler_init_fn_t    fcfs_scheduler_init;
static scheduler_add_fn_t     fcfs_scheduler_add;
static scheduler_remove_fn_t  fcfs_scheduler_remove;
static schedule_fn_t          fcfs_schedule;
static scheduler_dump_ready_vcpu_fn_t  fcfs_dump_ready_vcpu;

scheduler_t fcfs_scheduler = {
  {NULL, NULL, NULL, NULL},
  0,
  1,
  1, /* TODO */
  fcfs_scheduler_init,
  fcfs_scheduler_add,
  fcfs_scheduler_remove,
  fcfs_schedule,
  fcfs_dump_ready_vcpu,
};

#define MAX_PRIORITY 15

#define PRIORITY_NUM  MAX_PRIORITY+1

static struct {
  vcpu_t *head;
  vcpu_t *tail;
} ready_vcpu[PRIORITY_NUM];

/*
 * fcfs_scheduler_init() is called in scheduler_init()
 * after fcfs_scheduler.phys_cpu and fcfs_scheduler.pcpu%num are set.
 */
static void fcfs_scheduler_init(void){
  int i;

  for (i = 0; i < PRIORITY_NUM; i++) {
    ready_vcpu[i].head = NULL;
    ready_vcpu[i].tail = NULL;
  }
}

/* Add vcpu to the tail of ready que */
static void fcfs_scheduler_add(vcpu_t *vcpu){
  int i;
  vcpu_t *t_vcpu;

  if(vcpu->vm->priority > MAX_PRIORITY)
    hyp_panic("Illegal priority : %d\n", vcpu->vm->priority);

  /* Set schedule need flag of each cpu which uses this scheduler */
  for(i=0; i<fcfs_scheduler.pcpu_num; i++){
    fcfs_scheduler.phys_cpu[i]->schedule_is_needed = 1;
  }

  if(ready_vcpu[vcpu->vm->priority].head ==NULL)
    ready_vcpu[vcpu->vm->priority].head = vcpu;
  else
    ready_vcpu[vcpu->vm->priority].tail->next = vcpu;
  
  ready_vcpu[vcpu->vm->priority].tail = vcpu;
  vcpu->next = NULL;
}

static void fcfs_scheduler_remove(vcpu_t *vcpu){
  int i;
  vcpu_t *t_vcpu;
  
  log_debug("Remove a vcpu from fcfs scheduler's ready vcpu : vm name : %s, priority : %d\n",
      vcpu->vm->name, vcpu->vm->priority);

  if(ready_vcpu[vcpu->vm->priority].head == NULL){
    log_warn("You shoudn't try to remove vcpu which has already removed from  ready vcpu.\n");
    return;
  }

  if(vcpu == ready_vcpu[vcpu->vm->priority].head){
    ready_vcpu[vcpu->vm->priority].head = vcpu->next;
    if(vcpu == ready_vcpu[vcpu->vm->priority].tail)
      ready_vcpu[vcpu->vm->priority].tail = NULL;
  }else{
    for(t_vcpu = ready_vcpu[vcpu->vm->priority].head; ;  t_vcpu = t_vcpu->next){

      // This vcpu has already removed from  ready vcpu
      if(t_vcpu == ready_vcpu[vcpu->vm->priority].tail){
        log_warn("You shoudn't try to remove vcpu which has already removed from  ready vcpu.\n");
        return;
      }
      if(t_vcpu->next == vcpu)
        break;
    }
    //prev_vcpu->next = vcpu->next
    t_vcpu->next = t_vcpu->next->next;
    if(vcpu == ready_vcpu[vcpu->vm->priority].tail)
      ready_vcpu[vcpu->vm->priority].tail = t_vcpu;
  }

  vcpu->next = NULL;

  /* Set schedule need flag of each cpu which uses this scheduler */
  for(i=0; i<fcfs_scheduler.pcpu_num; i++){
    fcfs_scheduler.phys_cpu[i]->schedule_is_needed = 1;
  }

}

/* TODO : Support multi core */
static void fcfs_schedule(pcpu_t *phys_cpu){
  int i;

  phys_cpu->schedule_is_needed = 0;

  /* Find a vcpu with the highest priority in readyque */
  for (i = 0; i < PRIORITY_NUM; i++) {
    if (ready_vcpu[i].head != NULL)
      break;
  }
  
  /* Not found */
  if (i == PRIORITY_NUM)
    return;

  phys_cpu->current_vcpu = ready_vcpu[i].head; /* Set as current vcpu */
  
  /* Remove this vcpu from readyque */
  /* Do not use scheduler_remove for flags  */
  ready_vcpu[i].head = ready_vcpu[i].head->next;
  phys_cpu->current_vcpu->next = NULL;

  /* 
   * check whether the other physical cpu(s) which use this scheduler need to schedule,
   * If scheduling is needed to another physical cpu, 
   * cause interrupt on that cpu by smp mail box. 
   */
  for(i=0; i<fcfs_scheduler.pcpu_num; i++){
    if(fcfs_scheduler.phys_cpu[i]->schedule_is_needed)
      smp_send_mailbox(fcfs_scheduler.phys_cpu[i]->cpu_id, MAIL_TYPE_SCHEDULE);
  }
  
}

static void fcfs_dump_ready_vcpu(log_level_t level){
  int i;
  vcpu_t *t_vcpu;

  log_printf(level, "Start dump vcpu in fcfs scheduler ready que\n");
  
  for (i = 0; i < PRIORITY_NUM; i++) {
    if (ready_vcpu[i].head == NULL)
      continue;

    log_printf(level, "Priority level : %d\n", i);
    t_vcpu = ready_vcpu[i].head;
    while(t_vcpu != NULL){
      log_printf(level, "ready vm:%s vcpu_id:%d\n", t_vcpu->vm->name, t_vcpu->vcpu_id);
      t_vcpu = t_vcpu->next;
    }
  }
  log_printf(level, "=================   End   =================\n");
}
