/* 
 * virt_bcm2836_mailbox.c
 * Full virtualization mailboxes of bcm2836
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vcpu.h"
#include "virt_mmio.h"

/*
 * TODO
 * bcm2835 has Mailboxes for cpu-gpu communication.
 * 0x3F00b880 ~ 0x3F00b8bf
 */

#define BCM2835_BASE			0x3F00b880

static virt_mmio_reg_reset_fn_t bcm2835_mailbox_reg_reset;
static virt_mmio_reg_read_fn_t  bcm2835_mailbox_reg_read;
static virt_mmio_reg_write_fn_t bcm2835_mailbox_reg_write;

virt_full_mmio_reg_access_t bcm2835_mailbox_reg_access = {
  0x3F00b880, 0x3F00b8bf,
  bcm2835_mailbox_reg_reset,
  bcm2835_mailbox_reg_read,
  bcm2835_mailbox_reg_write,
  NULL,
  NULL,
};


static void bcm2835_mailbox_reg_reset(void){

  return;
}

static int bcm2835_mailbox_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){

  *(uint32_t *)dst = *(volatile uint32_t *)addr; 

  return 0;
}

static int bcm2835_mailbox_reg_write(vcpu_t *vcpu, 
              phys_addr_t addr, uint64_t value, uint8_t size){
   if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }
  
  *(volatile uint32_t *)addr = (uint32_t)value;
  
  return 0;
}

