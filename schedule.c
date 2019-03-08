/*
 * scheduler.c :
 *  manage schedulers
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vm.h"
#include "vcpu.h"
#include "schedule.h"

static int scheduler_locked = 0;;

scheduler_t *schedulers [] = {
  &fcfs_scheduler,
  &rr_scheduler,
  &no_scheduler,
};

void schedulers_init(void){
  int i;
  pcpu_t *t_phys_cpu;

  scheduler_locked = 0;

  /* Set scheduler's pointers to phys_cpu*/
  for(i=0; i<CPU_NUM; i++){
    t_phys_cpu = get_phys_cpu_by_cpu_id(i);
    if(t_phys_cpu->scheduler != NULL){
      t_phys_cpu->scheduler->phys_cpu[t_phys_cpu->scheduler->pcpu_num] = t_phys_cpu;
      t_phys_cpu->scheduler->pcpu_num++;
    }
    phys_cpus[i].schedule_is_needed = 0;
  }


  for(i = 0; i < sizeof(schedulers)/sizeof(schedulers[0]); i++){
    if(schedulers[i]->scheduler_init != NULL)
      schedulers[i]->scheduler_init();
  }
}

/* TODO : Separate scheduler_locked into each scheduler */
void do_schedule(pcpu_t *phys_cpu){
  spin_lock(&scheduler_locked);

  if(phys_cpu->schedule_is_needed){
    phys_cpu->scheduler->schedule(phys_cpu);
    if(phys_cpu->current_vcpu != NULL)
      vcpu_active(phys_cpu->current_vcpu, phys_cpu);
  }

  spin_unlock(&scheduler_locked);
}

void dump_ready_vcpu(log_level_t level){
  int i;
  for(i = 0; i < sizeof(schedulers)/sizeof(schedulers[0]); i++){
    if(schedulers[i]->dump_ready_vcpu != NULL)
      schedulers[i]->dump_ready_vcpu(level);
  }
}
