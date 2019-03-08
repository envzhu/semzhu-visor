/*
 * rr_scheduler.c :
 *  Round-robin scheduler
 */


#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vm.h"
#include "vcpu.h"
#include "hyp_timer.h"
#include "schedule.h"

static scheduler_init_fn_t    rr_scheduler_init;
static scheduler_add_fn_t     rr_scheduler_add;
static scheduler_remove_fn_t  rr_scheduler_remove;
static schedule_fn_t          rr_schedule_dummy;
static scheduler_dump_ready_vcpu_fn_t  rr_dump_ready_vcpu;

static void periodical_schedule(void);

scheduler_t rr_scheduler = {
  {NULL, NULL, NULL, NULL},
  0,
  0,
  0,
  rr_scheduler_init,
  rr_scheduler_add,
  rr_scheduler_remove,
  rr_schedule_dummy,
  rr_dump_ready_vcpu,
};

static struct {
  vcpu_t *head;
  vcpu_t *tail;
} ready_vcpu;

#define SCHEDULE_CYCLE_TIME_MSEC  100


/* 
 * rr_scheduler does not use phys_cpu->schedule_is_needed
 */
/*
 * rr_scheduler_init() is called in scheduler_init()
 * after rr_scheduler.phys_cpu and rr_scheduler.pcpu?num are set.
 */
static void rr_scheduler_init(void){
  int i;

  ready_vcpu.head = NULL;
  ready_vcpu.tail = NULL;

  for(i=0; i<rr_scheduler.pcpu_num; i++){
    timer_event_add(rr_scheduler.phys_cpu[i],
        periodical_schedule, SCHEDULE_CYCLE_TIME_MSEC, 0);
  }
}

static void rr_scheduler_register_phys_cpu(pcpu_t *phys_cpu){
  int i;

  /* Find a free pointer to phys_cpu */
  for(i = 0; i < sizeof(rr_scheduler.phys_cpu)/sizeof(rr_scheduler.phys_cpu[0]); i++){
    if(rr_scheduler.phys_cpu[i] == NULL)
      break;
  }

  /* If there is no free pointer */
  if(i == sizeof(rr_scheduler.phys_cpu)/sizeof(rr_scheduler.phys_cpu[0]))
    hyp_panic("You cannot register such a many phys_cpu to scheduler\n");

  rr_scheduler.phys_cpu[i] = phys_cpu;

}

/* Add vcpu to the tail of ready que */
static void rr_scheduler_add(vcpu_t *vcpu){
  vcpu_t *t_vcpu;

  if(ready_vcpu.head ==NULL)
    ready_vcpu.head = vcpu;
  else
    ready_vcpu.tail->next = vcpu;
  
  ready_vcpu.tail = vcpu;
}

static void rr_scheduler_remove(vcpu_t *vcpu){
  vcpu_t *t_vcpu;

  /* Remove from ready que */
  if(t_vcpu == ready_vcpu.head){
    ready_vcpu.head = t_vcpu->next;

  }else if(t_vcpu == ready_vcpu.tail){
    ready_vcpu.head = t_vcpu->next;

  }else{
    for(t_vcpu = ready_vcpu.head; ; t_vcpu->next){
      // This vcpu has already removed from  ready vcpu
      if(t_vcpu == ready_vcpu.tail)
        return;
      if(t_vcpu->next == vcpu)
        break;
    }
    //prev_vcpu->next = vcpu->next
    t_vcpu->next = t_vcpu->next->next;
  }

  vcpu->next = NULL;
}

static void rr_schedule_dummy(pcpu_t *phys_cpu){
  return;
}

static void rr_dump_ready_vcpu(log_level_t level){
  int i;
  vcpu_t *t_vcpu;

  log_printf(level, "Start dump vcpu in round robin scheduler ready que\n");

  t_vcpu = ready_vcpu.head;
  while(t_vcpu != NULL){
    log_printf(level, "ready vm:%s vcpu_id:%d\n", t_vcpu->vm->name, t_vcpu->vcpu_id);
    t_vcpu = t_vcpu->next;
  }

  log_printf(level, "=================   End   =================\n");
}

static void periodical_schedule(void){
  pcpu_t *phys_cpu = get_current_phys_cpu();

  timer_event_add(phys_cpu->cpu_id, periodical_schedule, SCHEDULE_CYCLE_TIME_MSEC, 0);
  
  /* Add last_vcpu to the tail of ready queue */
  vcpu_sleep(phys_cpu->last_vcpu);
  vcpu_ready(phys_cpu->last_vcpu);
  
  phys_cpu->current_vcpu = ready_vcpu.head; /* Set as current vcpu */
  
  /* Remove this vcpu from readyque */
  rr_scheduler_remove(phys_cpu->current_vcpu);
}
