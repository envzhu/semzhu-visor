#ifndef _SMP_MAILBOX_H_INCLUDED_
#define _SMP_MAILBOX_H_INCLUDED_

#include "typedef.h"

typedef enum {
  MAIL_TYPE_SCHEDULE= 1,
  MAIL_TYPE_TIMER_START,
  MAIL_TYPE_TIMER_STOP,
} smp_mail_type_t;

void smp_read_mailbox(uint8_t cpu_id);
void smp_cleaer_mailbox(uint8_t cpu_id);
void smp_send_mailbox(uint8_t cpu_id, uint32_t value);

#endif
