#ifndef _SCHEDULE_H_INCLUDED_
#define _SCHEDULE_H_INCLUDED_

typedef struct _scheduler_t scheduler_t;

#include "typedef.h"
#include "log.h"
#include "pcpu.h"
#include "vcpu.h"

typedef void (scheduler_init_fn_t)(void);
typedef void (scheduler_add_fn_t)(vcpu_t *vcpu);
typedef void (scheduler_remove_fn_t)(vcpu_t *vcpu);
typedef void (schedule_fn_t)(pcpu_t *phys_cpu);
typedef void (scheduler_dump_ready_vcpu_fn_t)(log_level_t level);

typedef struct _scheduler_t{
  pcpu_t *phys_cpu[CPU_NUM];
  int pcpu_num;
  int trap_sleep;
  int emulate_vtimer;
  scheduler_init_fn_t   *scheduler_init;
  scheduler_add_fn_t    *scheduler_add;
  scheduler_remove_fn_t *scheduler_remove;
  schedule_fn_t         *schedule;
  scheduler_dump_ready_vcpu_fn_t  *dump_ready_vcpu;
} scheduler_t;

extern scheduler_t fcfs_scheduler;
extern scheduler_t rr_scheduler;
extern scheduler_t no_scheduler;

void schedulers_init(void);
void do_schedule(pcpu_t *phys_cpu);
void dump_ready_vcpu(log_level_t level);

#endif
