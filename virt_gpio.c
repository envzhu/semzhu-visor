/* 
 * virt_gpio.c
 * Virtualization gpio regs
 * and exclusive manege each gpio pin
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "hardware_def.h"
#include "vcpu.h"
#include "virt_mmio.h"

#define GPIO_BASE   0x3F200000

#define GPFSEL0   (GPIO_BASE + 0x00)
#define GPFSEL1   (GPIO_BASE + 0x04)
#define GPFSEL2   (GPIO_BASE + 0x08)
#define GPFSEL3   (GPIO_BASE + 0x0C)
#define GPFSEL4   (GPIO_BASE + 0x10)
#define GPFSEL5   (GPIO_BASE + 0x14)
#define GPSET0    (GPIO_BASE + 0x1C)
#define GPSET1    (GPIO_BASE + 0x20)
#define GPCLR0    (GPIO_BASE + 0x28)
#define GPCLR1    (GPIO_BASE + 0x2c)
#define GPLEV0    (GPIO_BASE + 0x34)
#define GPLEV1    (GPIO_BASE + 0x38)
#define GPEDS0    (GPIO_BASE + 0x40)
#define GPEDS1    (GPIO_BASE + 0x44)
#define GPREN0    (GPIO_BASE + 0x4c)
#define GPREN1    (GPIO_BASE + 0x50)
#define GPFEN0    (GPIO_BASE + 0x58)
#define GPFEN1    (GPIO_BASE + 0x5c)
#define GPHEN0    (GPIO_BASE + 0x64)
#define GPHEN1    (GPIO_BASE + 0x68)
#define GPLEN0    (GPIO_BASE + 0x70)
#define GPLEN1    (GPIO_BASE + 0x74)
#define GPAREN0   (GPIO_BASE + 0x7c)
#define GPAREN1   (GPIO_BASE + 0x80)
#define GPAFEN0   (GPIO_BASE + 0x88)
#define GPAFEN1   (GPIO_BASE + 0x8c)
#define GPPUD     (GPIO_BASE + 0x94)
#define GPPUDCLK0 (GPIO_BASE + 0x98)
#define GPPUDCLK1 (GPIO_BASE + 0x9C)

static virt_mmio_reg_read_fn_t  gpio_reg_read;
static virt_mmio_reg_write_fn_t gpio_reg_write;
static virt_mmio_reg_assign_fn_t   gpio_assign;
static virt_mmio_reg_release_fn_t  gpio_release;

virt_excl_mmio_reg_access_t gpio_reg_access = {
  0x3F200000, 0x3F2000FF,
  NULL,
  gpio_reg_read,
  gpio_reg_write,
  gpio_assign,
  gpio_release,
};


static int gpio_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){
  int t_gpio_no;
  uint32_t t_reg;

  if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    /* 
     * These mmio registers are write-only,
     * So return -1;
     */
    case GPSET0:
    case GPSET1:
    case GPCLR0:
    case GPCLR1:
      log_error("Cannnot access to a write-only register.\n");
      break;

    case GPFSEL0:
    case GPFSEL1:
    case GPFSEL2:
    case GPFSEL3:
    case GPFSEL4:
    case GPFSEL5:
      for(t_gpio_no = (addr - GPFSEL0) / sizeof(uint32_t) * 10;
            t_gpio_no < ((addr - GPFSEL0) / sizeof(uint32_t) + 1) * 10;
            t_gpio_no++){
        *(volatile uint32_t *)addr >>= 3;
        if(((*(volatile uint32_t *)addr) & (0b111 << ((t_gpio_no%10)*3)))
            &&(vcpu->vm->assigned_gpio&(1<<t_gpio_no))){
          t_reg |=  (*(volatile uint32_t *)addr) & (0b111<<((t_gpio_no%10)*3));
        }
      }
      *(volatile uint32_t *)dst = t_reg;
      break;

    case GPPUD:
      break;
    
    /* access to the registers about GPIO0~GPIO31 */
    case GPLEV0:
    case GPEDS0:
    case GPREN0:
    case GPFEN0:
    case GPHEN0:
    case GPLEN0:
    case GPAREN0:
    case GPAFEN0:
    case GPPUDCLK0:
      *(uint32_t *)dst = (*(volatile uint32_t *)addr) & vcpu->vm->assigned_gpio;
      break;
    
    /* access to the registers about GPIO32~GPIO49 */
    case GPLEV1:
    case GPEDS1:
    case GPREN1:
    case GPFEN1:
    case GPHEN1:
    case GPLEN1:
    case GPAREN1:
    case GPAFEN1:
    case GPPUDCLK1:
      *(uint32_t *)dst = (*(volatile uint32_t *)addr) & (vcpu->vm->assigned_gpio >> 32);
      break;

    default:
      log_error("Illegal access to unavailable address\n");
      return -1;
  }

  return 0;
}

static int gpio_reg_write(vcpu_t *vcpu, 
              phys_addr_t addr, uint64_t value, uint8_t size){

  uint64_t t_gpio_no;
  uint32_t t_reg = 0;

  if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    /*
     * These register are read-only registers.
     * So return -1; 
     */
    case GPLEV0:
    case GPLEV1:
      log_error("Cannot access to a read-only register.\n");
      return -1;
      break;
    
    case GPFSEL0:
    case GPFSEL1:
    case GPFSEL2:
    case GPFSEL3:
    case GPFSEL4:
    case GPFSEL5:
      log_debug("Write ope to GPFSEL%d value : %#x, $pc:%#8x\n",
          (addr - GPFSEL0) / sizeof(uint32_t), value, vcpu->sysreg.pc);
      for(t_gpio_no = (addr - GPFSEL0) / sizeof(uint32_t) * 10;
            t_gpio_no < ((addr - GPFSEL0) / sizeof(uint32_t) + 1) * 10;
            t_gpio_no++){
        if((value & (0b111 << ((t_gpio_no%10)*3)))){
          if(vcpu->vm->assigned_gpio & ((uint64_t)1<<t_gpio_no)){
            t_reg |= value&(0b111<<((t_gpio_no%10)*3));
            /* Linux: bcm2835_pmx_set() */
            log_debug("Write ope to GPFSEL* in vm : %s,\n trited gpio No.: %d, value : %#x\n",
                vcpu->vm->name, t_gpio_no, (value>>(t_gpio_no%10)*3)&0b111);
          }else{
            /* TODO : Fix my poor English*/
            log_warn("Illegal write ope to GPFSEL* in vm : %s,\n    trited gpio No.: %d, assigned_gpio:%#8x\n",
                vcpu->vm->name, t_gpio_no, vcpu->vm->assigned_gpio);
          }
        }
      }
      *(volatile uint32_t *)addr = t_reg;
      break;
    
    /* TODO : Suppot GPIO pullup/down control */
    case GPPUD:
      break;
    /* access to the registers about GPIO0~GPIO31 */
    case GPSET0:
    case GPCLR0:
    case GPEDS0:
    case GPREN0:
    case GPFEN0:
    case GPHEN0:
    case GPLEN0:
    case GPAREN0:
    case GPAFEN0:
    case GPPUDCLK0:
      *(volatile uint32_t *)addr =  value & vcpu->vm->assigned_gpio;
      if(value & ~vcpu->vm->assigned_gpio)
        log_warn("Illegal gpio reg write ope in vm %s, tried value%#x, enabled value%#x\n",
          value, vcpu->vm->assigned_gpio&0xFFFFFFFF);
      break;
    
    /* access to the registers about GPIO32~GPIO49 */
    case GPSET1:
    case GPCLR1:
    case GPEDS1:
    case GPREN1:
    case GPFEN1:
    case GPHEN1:
    case GPLEN1:
    case GPAREN1:
    case GPAFEN1:
    case GPPUDCLK1:
      *(volatile uint32_t *)addr =  value & (vcpu->vm->assigned_gpio >> 32);
      if(value & ~(vcpu->vm->assigned_gpio >> 32))
        log_warn("Illegal gpio reg write ope in vm %s, tried value%#x, enabled value%#x\n",
          value, vcpu->vm->assigned_gpio >> 32);
      
      break;
      
    default:
      log_error("Illegal access to unavailable address\n");
      return -1;
  }

  return 0;
}


/* gpio_used bitmap */
static uint64_t gpio_used;

static int gpio_assign(vm_t *vm){
  
  if(gpio_used & vm->assigned_gpio){
    log_error("Failed to assign gpio; vm : %s, gpio_used bitmap : %#x, gpio_assigned bitmap : %#x\n",
        vm->name, gpio_used, vm->assigned_gpio);
    return -1;
  }
  
  gpio_used |= vm->assigned_gpio;
  
  log_info("Assign gpio; vm : %s, gpio_used bitmap : %#x, gpio_assigned bitmap : %#x\n",
      vm->name, gpio_used, vm->assigned_gpio);
  return 0;
}


static int gpio_release(vm_t *vm){
  
  gpio_used &= ~vm->assigned_gpio;

  log_info("Release gpio; vm : %s, gpio_used bitmap : %#x\n",
      vm->name, gpio_used);
  return 0;
}
