/* 
 * bcm2836_mailbox.c
 * Control bcm2836's mailbox
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vcpu.h"

/*
 * bcm2836 has 16 Mailboxes for cpu core communication.
 * Each cpu core has 4 mailboxes.
 * If a value is written to a mail box,
 * a IRQ interrupt wil be taken in the cpu core which that mailbox is assigned to 
 */

#define BCM2836_BASE			0x40000000

#define BCM2836_CORE0_MAILBOX0_SET  (BCM2836_BASE + 0x80)
#define BCM2836_CORE0_MAILBOX1_SET  (BCM2836_BASE + 0x84)
#define BCM2836_CORE0_MAILBOX2_SET  (BCM2836_BASE + 0x88)
#define BCM2836_CORE0_MAILBOX3_SET  (BCM2836_BASE + 0x8C)

#define BCM2836_CORE1_MAILBOX0_SET  (BCM2836_BASE + 0x90)
#define BCM2836_CORE1_MAILBOX1_SET  (BCM2836_BASE + 0x94)
#define BCM2836_CORE1_MAILBOX2_SET  (BCM2836_BASE + 0x98)
#define BCM2836_CORE1_MAILBOX3_SET  (BCM2836_BASE + 0x9C)

#define BCM2836_CORE2_MAILBOX0_SET  (BCM2836_BASE + 0xA0)
#define BCM2836_CORE2_MAILBOX1_SET  (BCM2836_BASE + 0xA4)
#define BCM2836_CORE2_MAILBOX2_SET  (BCM2836_BASE + 0xA8)
#define BCM2836_CORE2_MAILBOX3_SET  (BCM2836_BASE + 0xAC)

#define BCM2836_CORE3_MAILBOX0_SET  (BCM2836_BASE + 0xB0)
#define BCM2836_CORE3_MAILBOX1_SET  (BCM2836_BASE + 0xB4)
#define BCM2836_CORE3_MAILBOX2_SET  (BCM2836_BASE + 0xB8)
#define BCM2836_CORE3_MAILBOX3_SET  (BCM2836_BASE + 0xBC)

#define BCM2836_CORE0_MAILBOX0_RDCLR  (BCM2836_BASE + 0xC0)
#define BCM2836_CORE0_MAILBOX1_RDCLR  (BCM2836_BASE + 0xC4)
#define BCM2836_CORE0_MAILBOX2_RDCLR  (BCM2836_BASE + 0xC8)
#define BCM2836_CORE0_MAILBOX3_RDCLR  (BCM2836_BASE + 0xCC)

#define BCM2836_CORE1_MAILBOX0_RDCLR  (BCM2836_BASE + 0xD0)
#define BCM2836_CORE1_MAILBOX1_RDCLR  (BCM2836_BASE + 0xD4)
#define BCM2836_CORE1_MAILBOX2_RDCLR  (BCM2836_BASE + 0xD8)
#define BCM2836_CORE1_MAILBOX3_RDCLR  (BCM2836_BASE + 0xDC)

#define BCM2836_CORE2_MAILBOX0_RDCLR  (BCM2836_BASE + 0xE0)
#define BCM2836_CORE2_MAILBOX1_RDCLR  (BCM2836_BASE + 0xE4)
#define BCM2836_CORE2_MAILBOX2_RDCLR  (BCM2836_BASE + 0xE8)
#define BCM2836_CORE2_MAILBOX3_RDCLR  (BCM2836_BASE + 0xEC)

#define BCM2836_CORE3_MAILBOX0_RDCLR  (BCM2836_BASE + 0xF0)
#define BCM2836_CORE3_MAILBOX1_RDCLR  (BCM2836_BASE + 0xF4)
#define BCM2836_CORE3_MAILBOX2_RDCLR  (BCM2836_BASE + 0xF8)
#define BCM2836_CORE3_MAILBOX3_RDCLR  (BCM2836_BASE + 0xFC)


uint32_t bcm2836_mailbox_reg_cpu_core_read(uint8_t cpu_id, uint8_t mbox_id){
  return *(volatile uint32_t *)((cpu_id << 4) + mbox_id*4 + BCM2836_CORE0_MAILBOX0_RDCLR);
}

void bcm2836_mailbox_reg_cpu_core_clear(uint8_t cpu_id, uint8_t mbox_id){
  *(volatile uint32_t *)((cpu_id << 4) + mbox_id*4 + BCM2836_CORE0_MAILBOX0_RDCLR)
      = 0xFFFFFFFF;
}

void bcm2836_mailbox_reg_cpu_core_write(uint8_t cpu_id, uint8_t mbox_id, uint32_t value){
  bcm2836_mailbox_reg_cpu_core_clear(cpu_id, mbox_id);
  *(volatile uint32_t *)((cpu_id << 4) + mbox_id*4 + BCM2836_CORE0_MAILBOX0_SET)
      = value;
}

