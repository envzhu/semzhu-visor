/*
 * pcpu.h
 * Physical CPU Control Block
 */

#ifndef _PCPU_H_INCLUDED_
#define _PCPU_H_INCLUDED_

typedef struct _pcpu_t pcpu_t;
#define CPU_NUM   4

#include "typedef.h"
#include "vcpu.h"
#include "schedule.h"

typedef struct _pcpu_t{
  uint32_t cpu_id;
  vcpu_t *last_vcpu;
  vcpu_t *current_vcpu;
  uint64_t freq;
  uint64_t intr_state;
  int schedule_is_needed;
  scheduler_t *scheduler;
} pcpu_t;

extern pcpu_t phys_cpus[CPU_NUM];

pcpu_t *current_phys_cpu_core_init(void);
pcpu_t *get_current_phys_cpu(void);
pcpu_t *get_phys_cpu_by_cpu_id(uint8_t cpu_id);

#endif
