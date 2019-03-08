#include "typedef.h"
#include "lib.h"
#include "print.h"
#include "asm_func.h"
#include "log.h"

#define LOG_NUM_MAX 0x40000
static uint64_t count = 0;
static int log_level = 8;

char *log_level_msg[] = {
  "[Critical] ",
  "[Error] ",
  "[Warning] ",
  " ",
  " ",
};

void log_level_set(log_level_t level){
  log_level = level;
}

static int log_vprintf(log_level_t level , char *fmt, uint64_t *argv, int argc){
  int ret;

  if(level > log_level)
    return -1;

  printf("[%#3x]%s", count, log_level_msg[level]);
  
  ret = vprintf(fmt, &argv[0], argc);

  if(count >= LOG_NUM_MAX)
    hyp_panic("There are too many log.\n");
  
  count++;
  return ret;
}

int log_printf(log_level_t level, char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(level, fmt, &argv[2], 6);
}

int log_crit(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(LOG_CRIT, fmt, &argv[1], 7);
}

int log_error(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(LOG_ERROR, fmt, &argv[1], 7);
}

int log_warn(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(LOG_WARN, fmt, &argv[1], 7);
}

int log_info(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(LOG_INFO, fmt, &argv[1], 7);
}


int log_debug(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  return log_vprintf(LOG_DEBUG, fmt, &argv[1], 7);
}


void hyp_panic(char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);

  printf("\n[Hypervisor Panic!]");
  vprintf(fmt, &argv[1], 7);
  printf("Stop this cpu!\n");

  /* TODO : Also stop another cpus */
  cpu_hlt();
}
