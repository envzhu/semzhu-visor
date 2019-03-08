/* 
 * hyp_timer.c
 * Control PMU(Performance Monitor Unit)
 */

#include "typedef.h"
#include "asm_func.h"
#include "hyp_timer.h"
#include "pmu.h"

/*
  PMU registers: 
  PMCR_EL0      RW 32 PMU Control Reg
  PMCNTENSET_EL0  RW 32 PM Count Enable Set Reg
  PMCNTENCLR_EL0  RW 32 PM Count Enable Clear Reg
  PMOVSCLR_EL0    RW 32 PM Overflow Flag Status Reg
  PMSWINC_EL0   WO 32 PM Software Increment Reg
  PMSELR_EL0    RW 32 PM Event Counter Selection Reg
  PMCEID0_EL0   RO 32 PM Common Event Identification Reg 0
  PMCEID1_EL0   RO 32 PM Common Event Identification Reg 1
  PMCCNTR_EL0   RW 64 PM Cycle Count Reg
  PMXEVTYPER_EL0  RW 32 PM Selected Event Type and Filter Reg
  PMCCFILTR_EL0   RW 32 PM Cycle Count Filter Reg
  PMXEVCNTR0_EL0  RW 32 PM Selected Event Count Reg
  PMUSERENR_EL0   RW 32 PM User Enable Reg
  PMINTENSET_EL1  RW 32 PM Interrupt Enable Set Reg
  PMINTENCLR_EL1  RW 32 PM Interrupt Enable Clear Reg
  PMOVSSET_EL0  RW 32 PM Overflow Flag Status Set Reg
  PMEVCNTRx_EL0 RW 32 PM Event Count Regs
 */

/*
 * PM config registers:
 * 
 * PMCR_EL0     PM Control Reg
 * PMOVSCLR_EL0 : PM Overflow Flag Status Clear Reg
 * PMOVSSET_EL0 : PM Overflow Flag Status Set Reg
 * PMUSERENR_EL0  : User Enable Reg
 * PMINTENSET_EL1 : Interrupt Enable Reg
 * PMINTENCLR_EL1 : Interrupt Disable Reg
 */

/*
 * PM cycle counter registers:
 * 
 * PMCR_EL0     PM Control Reg
 * PMCNTENSET_EL0 : conunt enable set reg
 * PMCNTENCLR_EL0 : conunt enable clear reg
 * PMCCFILTR_EL0  : Cycle Count Filter Reg
 * PMCCNTR_EL0  : cycle count value reg
 */

/*
 * On Cortex-A53, there is 6 event counters.
 * PM event counter registers:
 * 
 * PMCR_EL0   : PM Control Reg
 * PMCNTENSET_EL0 : conunt enable set reg
 * PMCNTENCLR_EL0 : conunt enable clear reg
 * PMSELR_EL0     : Event Counter Selection Reg
 * PMCEID0_EL0    : Common Event Identification Reg 0
 * PMCEID1_EL0    : Common Event Identification Reg 1
 * PMXEVTYPER_EL0 : Selected Event Type and Filter Reg
 * PMXEVCNTR0_EL0 : Selected Event Count Reg
 * PMSWINC_EL0    : Software Increment Reg
 * PMCCFILTR_EL0  : Cycle Count Filter Reg
 * PMEVCNTRx_EL0  : Event Counter x value Regs (x = 0 ~ 5)
 */


// TODO :  WRITE_SYSREG(pmccfiltr_el0, (1<<27));
inline static void pmu_enable(void);
inline static void pmu_disable(void);

void cycle_counter_enable(void);
void cycle_counter_reset(void);
uint64_t cycle_counter_read(void);
void cycle_counter_disable(void);

void cycle_count_start(void){
  pmu_enable();
  cycle_counter_reset();
  cycle_counter_enable();
}

uint64_t cycle_count_stop(void){
  cycle_counter_disable();
  return cycle_counter_read();
}

inline static void pmu_enable(void){
  uint32_t r;
  READ_SYSREG(r, pmcr_el0);
  r |= 1;
  WRITE_SYSREG(pmcr_el0, r);
}

inline static void pmu_disable(void){
  uint32_t r;
  READ_SYSREG(r, pmcr_el0);
  r &= ~(uint32_t)1;
  WRITE_SYSREG(pmcr_el0, r);
}

void cycle_counter_enable(void){
  uint32_t r;

  READ_SYSREG(r, pmcntenset_el0);
  r |= (1<<31);
  WRITE_SYSREG(pmcntenset_el0, r);
}

void cycle_counter_disable(void){
  uint32_t r;

  READ_SYSREG(r, pmcntenclr_el0);
  r |= (1<<31);
  WRITE_SYSREG(pmcntenclr_el0, r);
}

uint64_t cycle_counter_read(void){
  uint64_t r;
  
  READ_SYSREG(r, pmccntr_el0);
  return r;
}

void cycle_counter_reset(void){
  uint32_t r;

  READ_SYSREG(r, pmcr_el0);
  r |= (1<<2);
  WRITE_SYSREG(pmcr_el0, r);
}

static void dump_cpu_usage_timer_event(pcpu_t *phys_cpu);
void dump_cpu_usage_start(pcpu_t *phys_cpu){
  timer_event_add(phys_cpu, dump_cpu_usage_timer_event, 1000, 0);
  cycle_count_start();
}

static void dump_cpu_usage_timer_event(pcpu_t *phys_cpu){
  cycle_count_stop();
  
  log_info("cpu cycle count   : %#8d\n", cycle_counter_read());  
  log_info("timer cycle count : %#8d\n", hyp_timer_get_clocks_num(1000));  

  log_info("cpu usage : %d%%\n", 100*cycle_counter_read()/hyp_timer_get_clocks_num(1000));
  cycle_count_start();
  
  timer_event_add(phys_cpu, dump_cpu_usage_timer_event, 1000, 0);
}

void dump_cpu_usage_stop(void){
  timer_event_remove(get_current_phys_cpu(), dump_cpu_usage_timer_event);
}
