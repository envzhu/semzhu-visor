/* 
 * virt_bcm2836_cprman.c
 * Full virtualization clock driver of bcm2835
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vcpu.h"
#include "virt_mmio.h"

/*
  TODO
 */

#define BCM2835_BASE			0x3F101000

static virt_mmio_reg_read_fn_t  bcm2835_cprman_reg_read;
static virt_mmio_reg_write_fn_t bcm2835_cprman_reg_write;
static virt_mmio_reg_reset_fn_t bcm2835_cprman_reg_reset;

virt_full_mmio_reg_access_t bcm2835_cprman_reg_access = {
  0x3F101000, 0x3F102FFF,
  bcm2835_cprman_reg_reset,
  bcm2835_cprman_reg_read,
  bcm2835_cprman_reg_write,
  NULL,
  NULL,
};

static void bcm2835_cprman_reg_reset(void){

  return;
}

static int bcm2835_cprman_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){
  
  log_debug("cpramn_read()\n");
  *(volatile uint32_t *)dst = 0;
  return 0;
}

static int bcm2835_cprman_reg_write(vcpu_t *vcpu, 
              phys_addr_t addr, uint64_t value, uint8_t size){

  log_debug("cpramn_write()\n");
  return 0;
}

