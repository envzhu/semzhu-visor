#ifndef _HYP_TIMER_H_INCLUDED_
#define _HYP_TIMER_H_INCLUDED_

#include "typedef.h"
#include "pcpu.h"

void hyp_timer_core_init(pcpu_t *phys_cpu);
uint64_t hyp_timer_get_clocks_num(int64_t msec);
uint64_t hyp_timer_tick2msec(int64_t tick);
void hyp_timer_intr(pcpu_t *phys_cpu);
void timer_event_init(void);
void timer_event_add(pcpu_t *phys_cpu,
    void (*func)(pcpu_t *phys_cpu, uint64_t arg), int64_t msec, uint64_t arg);
void timer_event_remove(pcpu_t *phys_cpu, void (*func) (void));

#endif
