/*
 * phys_cpu_setting.c :
 *  Describe the setting of physical cpu such as scheduler 
 */


#include "typedef.h"
#include "lib.h"
#include "schedule.h"
#include "pcpu.h"

pcpu_t phys_cpus[CPU_NUM] = {
  {
    .cpu_id = 0,
    .scheduler = &fcfs_scheduler,
  },
  {
    .cpu_id = 1,
    .scheduler = &rr_scheduler,
  },
  {
    .cpu_id = 2,
    .scheduler = &fcfs_scheduler,
  },
  {
    .cpu_id = 3,
    .scheduler = &fcfs_scheduler,
  },
};
