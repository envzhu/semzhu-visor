#ifndef _LOG_H_INCLUDED_
#define _LOG_H_INCLUDED_

#include "typedef.h"

typedef enum {
  LOG_CRIT = 0,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,
  LOG_LEVEL_MAX,
}log_level_t;

void log_level_set(log_level_t level);
int log_printf(log_level_t level, char *fmt, ...);
int log_crit(char *fmt, ...);
int log_error(char *fmt, ...);
int log_warn(char *fmt, ...);
int log_info(char *fmt, ...);
int log_debug(char *fmt, ...);
void hyp_panic(char *fmt, ...);


static inline uint32_t extract(uint32_t value, int start, int end)
{ 
  return (value & ((1<<(end+1)) - 1)) >> start;
}

#endif
