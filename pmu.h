#ifndef _PMU_H_INCLUDED_
#define _PMU_H_INCLUDED_

#include "typedef.h"
#include "pcpu.h"

void cycle_count_start(void);
uint64_t cycle_counter_read(void);
uint64_t cycle_count_stop(void);
void dump_cpu_usage_start(pcpu_t *phys_cpu);
void dump_cpu_usage_stop(void);

#endif
