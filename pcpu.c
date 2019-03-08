#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "vcpu.h"
#include "pcpu.h"

/* defined in phys_cpu_setting.c */
extern pcpu_t phys_cpus[CPU_NUM];

/* TODO */
pcpu_t *current_phys_cpu_core_init(void){
  uint32_t cpu_id;
  pcpu_t *phys_cpu;
  
  READ_CPUID(cpu_id);
  if(cpu_id >= CPU_NUM)
    hyp_panic("illegal cpu id : %#x\n", cpu_id);

  phys_cpu = &phys_cpus[cpu_id];
  phys_cpu->cpu_id = cpu_id;
  phys_cpu->current_vcpu = NULL;
  phys_cpu->last_vcpu = NULL;


  READ_SYSREG(phys_cpu->freq, CNTFRQ_EL0);

  return phys_cpu;
}


pcpu_t *get_current_phys_cpu(void){
  uint32_t cpu_id;
  READ_CPUID(cpu_id);
  
  return &phys_cpus[cpu_id];
}

pcpu_t *get_phys_cpu_by_cpu_id(uint8_t cpu_id){
  return &phys_cpus[cpu_id];
}

void *phys_cpu_relax(pcpu_t *phys_cpu){
  
}
