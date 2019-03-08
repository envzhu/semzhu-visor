#ifndef _VIRT_MMIO_H_INCLUDED_
#define _VIRT_MMIO_H_INCLUDED_

#include "typedef.h"
#include "vm.h"

/*
 * Peripherals Address Map
 * 
 * AUX    0x3F215000
 * PL011  0x3F201000
 * BSC0   0x3F205000
 * BSC1   0x3F804000
 * BSC2   0x3F805000
 * DMA    0x3F007000
 * DMA15  0x3FE05000
 * EMMC   0x3F300000
 * GPIO   0x3F200000
 * CM_GP  0x3F101000
 * INTR   0x3F00B000
 * I2S    0x3F203000
 * PWM    0x3F20C000
 * SPI    0x3F204000
 * SPIx   0x3F215000
 * SYSTMR 0x3F003000
 * USB    0x3F980000
 * CPRMAN 0x3F101000
 */

typedef enum {
  MMIO_SYSTMR = 1,
  MMIO_GPIO,
  MMIO_PL011,
  MMIO_I2S,
  MMIO_PWM,
  MMIO_SPI,
  MMIO_AUX,
  MMIO_CM_GP,
  MMIO_EMMC,
  MMIO_DMA,
  MMIO_DMA15,
  MMIO_BSC0,
  MMIO_BSC1,
  MMIO_BSC2,
  MMIO_USB,
  MMIO_NUM
} device_type_t;

#define VIRT_MMIO_SYSTMR  (1<<MMIO_SYSTMR)
#define VIRT_MMIO_GPIO  (1<<MMIO_GPIO)
#define VIRT_MMIO_PL011 (1<<MMIO_PL011)
#define VIRT_MMIO_I2S   (1<<MMIO_I2S)
#define VIRT_MMIO_PWM   (1<<MMIO_PWM)
#define VIRT_MMIO_SPI   (1<<MMIO_SPI)
#define VIRT_MMIO_AUX   (1<<MMIO_AUX)
#define VIRT_MMIO_CM_GP (1<<MMIO_CM_GP)
#define VIRT_MMIO_EMMC  (1<<MMIO_EMMC)
#define VIRT_MMIO_DMA   (1<<MMIO_DMA)
#define VIRT_MMIO_DMA15 (1<<MMIO_DMA15)
#define VIRT_MMIO_BSC0  (1<<MMIO_BSC0)
#define VIRT_MMIO_BSC1  (1<<MMIO_BSC1)
#define VIRT_MMIO_BSC2  (1<<MMIO_BSC2)
#define VIRT_MMIO_USB   (1<<MMIO_USB)

void excl_mmio_assign(vm_t *vm, uint64_t excl_mmio_assigned);

typedef void (virt_mmio_reg_reset_fn_t)(void);
typedef int (virt_mmio_reg_read_fn_t)
    (vcpu_t *vcpu, phys_addr_t addr, void *dst, uint8_t size);
typedef int (virt_mmio_reg_write_fn_t)
    (vcpu_t *vcpu, phys_addr_t addr, uint64_t value, uint8_t size);

typedef void (virt_mmio_reg_context_save_fn_t)(vcpu_t *vcpu);
typedef void (virt_mmio_reg_context_restore_fn_t)(vcpu_t *vcpu);

typedef int (virt_mmio_reg_assign_fn_t)(vm_t *vm);
typedef int (virt_mmio_reg_release_fn_t)(vm_t *vm);

/* access bolck for full virtualization mmio */
typedef struct _virt_full_mmio_reg_access_t{
  phys_addr_t mem_start;
  phys_addr_t mem_end;
  virt_mmio_reg_reset_fn_t  *reg_reset_fn;
  virt_mmio_reg_read_fn_t   *reg_read_fn;
  virt_mmio_reg_write_fn_t  *reg_write_fn;
  virt_mmio_reg_context_save_fn_t   *reg_save_fn;
  virt_mmio_reg_context_restore_fn_t  *reg_restore_fn;
} virt_full_mmio_reg_access_t;

/* access bolck for exclusive virtualization mmio */
typedef struct _virt_excl_mmio_reg_access_t{
  phys_addr_t mem_start;
  phys_addr_t mem_end;
  virt_mmio_reg_reset_fn_t  *reg_reset_fn;
  virt_mmio_reg_read_fn_t   *reg_read_fn;
  virt_mmio_reg_write_fn_t  *reg_write_fn;
  virt_mmio_reg_assign_fn_t *reg_assign_fn;
  virt_mmio_reg_release_fn_t  *reg_release_fn;
} virt_excl_mmio_reg_access_t;

void virt_mmio_reg_reset(void);

int virt_mmio_reg_access(vcpu_t *vcpu, uint64_t opcode, phys_addr_t reg_addr, int rw);

void virt_mmio_reg_context_save(vcpu_t *vcpu);
void virt_mmio_reg_context_restore(vcpu_t *vcpu);

void virt_mmio_reg_assign(vm_t *vm);
void virt_mmio_reg_release(vm_t *vm);

#endif
