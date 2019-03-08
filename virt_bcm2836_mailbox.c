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


static virt_mmio_reg_read_fn_t  bcm2836_mailbox_reg_read;
static virt_mmio_reg_write_fn_t bcm2836_mailbox_reg_write;

virt_full_mmio_reg_access_t bcm2836_mailbox_reg_access = {
  0x40000080, 0x400000FF,
  NULL,
  bcm2836_mailbox_reg_read,
  bcm2836_mailbox_reg_write,
  NULL,
  NULL,
};

static int bcm2836_mailbox_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){
  
  if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  if( ((addr&0x70)>>4) >= vcpu->vm->vcpu_num ){
    log_error("You cannot access bcm2836 mailbox registers which is not assigned to  the vm.\n");
    return -1;
  }
  
  vcpu_t *src_vcpu = vcpu->vm->vcpu[(addr&0x70)>>4];

  switch(addr){
    case BCM2836_CORE0_MAILBOX0_SET :
    case BCM2836_CORE0_MAILBOX1_SET :
    case BCM2836_CORE0_MAILBOX2_SET :
    case BCM2836_CORE0_MAILBOX3_SET :
    case BCM2836_CORE1_MAILBOX0_SET :
    case BCM2836_CORE1_MAILBOX1_SET :
    case BCM2836_CORE1_MAILBOX2_SET :
    case BCM2836_CORE1_MAILBOX3_SET :
    case BCM2836_CORE2_MAILBOX0_SET :
    case BCM2836_CORE2_MAILBOX1_SET :
    case BCM2836_CORE2_MAILBOX2_SET :
    case BCM2836_CORE2_MAILBOX3_SET :
    case BCM2836_CORE3_MAILBOX0_SET :
    case BCM2836_CORE3_MAILBOX1_SET :
    case BCM2836_CORE3_MAILBOX2_SET :
    case BCM2836_CORE3_MAILBOX3_SET :
    
      /* 
       * These registers are write only.
       * So we return -1.
       */
      return -1;
      break;

    case BCM2836_CORE0_MAILBOX0_RDCLR :
    case BCM2836_CORE0_MAILBOX1_RDCLR :
    case BCM2836_CORE0_MAILBOX2_RDCLR :
    case BCM2836_CORE0_MAILBOX3_RDCLR :
    case BCM2836_CORE1_MAILBOX0_RDCLR :
    case BCM2836_CORE1_MAILBOX1_RDCLR :
    case BCM2836_CORE1_MAILBOX2_RDCLR :
    case BCM2836_CORE1_MAILBOX3_RDCLR :
    case BCM2836_CORE2_MAILBOX0_RDCLR :
    case BCM2836_CORE2_MAILBOX1_RDCLR :
    case BCM2836_CORE2_MAILBOX2_RDCLR :
    case BCM2836_CORE2_MAILBOX3_RDCLR :
    case BCM2836_CORE3_MAILBOX0_RDCLR :
    case BCM2836_CORE3_MAILBOX1_RDCLR :
    case BCM2836_CORE3_MAILBOX2_RDCLR :
    case BCM2836_CORE3_MAILBOX3_RDCLR :
    
      *(uint32_t *)dst = src_vcpu->vic.core_mbox[(addr&0xf)/4];
      break;

    default:
      log_error("Illegal access to unavailable address\n");
      return -1;
  }

  return 0;
}

static int bcm2836_mailbox_reg_write(vcpu_t *vcpu, 
              phys_addr_t addr, uint64_t value, uint8_t size){
 
  if(size != 32){
    log_debug("Support read or write mmio only in 32bit.\n");
    return -1;
  }
 
  if( ((addr&0x70)>>4) >= vcpu->vm->vcpu_num ){
    log_error("You cannot access bcm2836 mailbox registers which is not assigned to  the vm.\n");
    return -1;
  }
  
  vcpu_t *src_vcpu = vcpu->vm->vcpu[(addr&0x70)>>4];

  switch(addr){
    case BCM2836_CORE0_MAILBOX0_SET :
    case BCM2836_CORE0_MAILBOX1_SET :
    case BCM2836_CORE0_MAILBOX2_SET :
    case BCM2836_CORE0_MAILBOX3_SET :
    case BCM2836_CORE1_MAILBOX0_SET :
    case BCM2836_CORE1_MAILBOX1_SET :
    case BCM2836_CORE1_MAILBOX2_SET :
    case BCM2836_CORE1_MAILBOX3_SET :
    case BCM2836_CORE2_MAILBOX0_SET :
    case BCM2836_CORE2_MAILBOX1_SET :
    case BCM2836_CORE2_MAILBOX2_SET :
    case BCM2836_CORE2_MAILBOX3_SET :
    case BCM2836_CORE3_MAILBOX0_SET :
    case BCM2836_CORE3_MAILBOX1_SET :
    case BCM2836_CORE3_MAILBOX2_SET :
    case BCM2836_CORE3_MAILBOX3_SET :
      
      src_vcpu->vic.core_mbox[(addr&0xf)/4] |= value;
      vcpu_do_virq(src_vcpu);
      break;
    
    case BCM2836_CORE0_MAILBOX0_RDCLR :
    case BCM2836_CORE0_MAILBOX1_RDCLR :
    case BCM2836_CORE0_MAILBOX2_RDCLR :
    case BCM2836_CORE0_MAILBOX3_RDCLR :
    case BCM2836_CORE1_MAILBOX0_RDCLR :
    case BCM2836_CORE1_MAILBOX1_RDCLR :
    case BCM2836_CORE1_MAILBOX2_RDCLR :
    case BCM2836_CORE1_MAILBOX3_RDCLR :
    case BCM2836_CORE2_MAILBOX0_RDCLR :
    case BCM2836_CORE2_MAILBOX1_RDCLR :
    case BCM2836_CORE2_MAILBOX2_RDCLR :
    case BCM2836_CORE2_MAILBOX3_RDCLR :
    case BCM2836_CORE3_MAILBOX0_RDCLR :
    case BCM2836_CORE3_MAILBOX1_RDCLR :
    case BCM2836_CORE3_MAILBOX2_RDCLR :
    case BCM2836_CORE3_MAILBOX3_RDCLR :
      
      src_vcpu->vic.core_mbox[(addr&0xf)/4] &= (~value);
      vcpu_do_virq(src_vcpu);
      break;
    
    default:
      log_error("Illegal access to unavailable address\n");
      return -1;
  }
  
  if(src_vcpu->vic.core_mbox[(addr&0xf)/4]){
    /* TODO : Set a interrupt pending flag */
    vcpu_do_virq(src_vcpu);
  }

  return 0;
}
