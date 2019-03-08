#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "vcpu.h"
#include "smp_mbox.h"

#define DEFAULT_MAILBOX_ID 0


void smp_read_mailbox(uint8_t cpu_id){
  //cpu_mailbox_read(cpu_id, DEFAULT_MAILBOX_ID);
}

void smp_cleaer_mailbox(uint8_t cpu_id){
  //cpu_mailbox_clear(cpu_id, DEFAULT_MAILBOX_ID);
}

void smp_send_mailbox(uint8_t cpu_id, smp_mail_type_t value){
  //cpu_mailbox_write(cpu_id, DEFAULT_MAILBOX_ID, (uint32_t)value);
}
