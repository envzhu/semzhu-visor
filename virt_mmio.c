/* 
 * virt_mmio.c
 * Virtualization peripheral device in mmio
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vcpu.h"
#include "hyp_mmu.h"
#include "virq.h"
#include "virt_mmio.h"

/*
 * Exclusive MMIO devices management
 */

/* Exclusive management block */
typedef struct _exclusive_mmio_info_t{
  phys_addr_t mem_start;
  phys_addr_t mem_length;
  vm_t *vm;
} exclusive_mmio_info_t;

/* 
 *These devices are not virtualized,
 * only are maneged exclusively by hypervisor.
 */
exclusive_mmio_info_t exclusive_devices[] = {
  {0, 0, 0}, /* reserved */
  {0x3F003000, 0x1000, NULL}, /* SYSTMR */
  {0x3F200000, 0x1000, NULL}, /* GPIO */
  {0x3F201000, 0x1000, NULL}, /* PL011 */
  {0x3F203000, 0x1000, NULL}, /* I2S */
  {0x3F20C000, 0x1000, NULL}, /* PWM */
  {0x3F204000, 0x1000, NULL}, /* SPI */
  {0x3F215000, 0x1000, NULL}, /* AUX (SPIx UART) */
  {0x3F101000, 0x1000, NULL}, /* CM_GP */
  {0x3F300000, 0x1000, NULL}, /* EMMC */
  {0x3F007000, 0x1000, NULL}, /* DMA */
  {0x3FE05000, 0x1000, NULL}, /* DMA15 */
  {0x3F205000, 0x1000, NULL}, /* BSC0 */
  {0x3F804000, 0x1000, NULL}, /* BSC1 */
  {0x3F805000, 0x1000, NULL}, /* BSC2 */
  {0x3F980000, 0x1000, NULL}, /* USB */
  {0x3F101000, 0x2000, NULL}, /* CPRMAN */
};

void excl_mmio_assign(vm_t *vm, uint64_t excl_mmio_assigned){
  int i;
  log_info("excl_mmio_assigned %#x\n", excl_mmio_assigned);
  
  for(i=1; i<sizeof(exclusive_devices)/sizeof(exclusive_devices[0]); i++){
    if((excl_mmio_assigned>>i)&1){
      
      if(exclusive_devices[i].vm != NULL)
        hyp_panic("This mmio device is already assigned to another vm!\n");

      map_page_table(vm->vttbr, exclusive_devices[i].mem_start, exclusive_devices[i].mem_start,
          exclusive_devices[i].mem_length);
      log_debug("assigned exclusively managed MMIO id %d, addr : %#8x\n", i, exclusive_devices[i].mem_start);
    }
  }
}

/* TODO */
#if 0
void exclusive_mmio_release(vm_t *vm){
  int i;
  
  for(i=1; i<sizeof(exclusive_devices)/sizeof(exclusive_devices[0]); i++){
    
    exclusive_devices[i].vm = NULL;
    map_page_free(vm->vttbr, exclusive_devices[i].mem_start, exclusive_devices[i].mem_start);
  }
}
#endif

#if 0

int decode_opcodae(uint32_t opcode){
  log_debug("instruction code : %#x\n", opcode);
  int imm;
  uint8_t size, Rm, Rn, Rt;

  Rt = opcode&0b11111;
  Rn = (opcode>>5)&0b11111;
  
  /* get immediate value */
  if((opcode>>21&0b11001)==0b00000){
    imm = (opcode>>12) & 0xff;
    if(opcode&(1<<20))
      imm = 0 - (0x100 - imm);
  } else if((opcode>>22&0b1100)==0b0100){
    imm = (opcode>>10) & 0xfff;
  }

  // if pre index
  if((((opcode>>24)&0b11)==0b00)&&(((opcode>>10)&0b11)==0b11))
    vcpu->reg.x[Rn] += imm;
  
  if(((opcode>>22)&0b0011110011)==0b0011100001){
    log_debug("Load Instruction\n");

    if((opcode>>27)==0b11111){
      log_debug("LDR 64bit\n");
    } else if((opcode>>21)&0b11001==0b00001){
      log_debug("LDR w%d, [x%d] \n", Rt, Rn);
      
      log_debug("x%d : %#x,  [] : %#x\n", Rn, vcpu->reg.x[Rn], *(uint32_t *)vcpu->reg.x[Rn]);
      vcpu->reg.x[Rt] = *(uint32_t *)vcpu->reg.x[Rn];
    } else if((opcode>>22)==0b1011100101){
      log_debug("LDR w%d, [x%d] \n", Rt, Rn);
      
      log_debug("x%d : %#x,  [] : %#x\n", Rn, vcpu->reg.x[Rn], *(uint32_t *)vcpu->reg.x[Rn]);
      vcpu->reg.x[Rt] = *(uint32_t *)vcpu->reg.x[Rn];
    }
  } else if(((opcode)>>22&0b0011110011)==0b0011100000){
    if((opcode>>22)==0b1111100000){
      log_debug("STR or STUR\n");
    } else if((opcode>>22)==0b1111100100){
      log_debug("STR\n");
    }else if((opcode>>22)==0b1011100100){
      log_debug("STR w%d, [x%d] \n", Rt, Rn);
      
      log_debug("x%d : %#x,  w%d : %#x\n", Rn, vcpu->reg.x[Rn], Rt, vcpu->reg.x[Rt]);
      *(uint32_t *)vcpu->reg.x[Rn] = (uint32_t)vcpu->reg.x[Rt];
    }
  }
    // if post index
    if((((opcode>>24)&0b11)==0b00)&&(((opcode>>10)&0b11)==0b01))
      vcpu->reg.x[Rn] += imm;
}
#endif
extern virt_full_mmio_reg_access_t bcm2836_ic_reg_access;
extern virt_full_mmio_reg_access_t bcm2835_mailbox_reg_access;
extern virt_full_mmio_reg_access_t bcm2835_cprman_reg_access;
extern virt_excl_mmio_reg_access_t bcm2835_ic_reg_access;
extern virt_excl_mmio_reg_access_t gpio_reg_access;
/* Full virtualization mmio list */
virt_full_mmio_reg_access_t *virt_full_devices[] = {
  &bcm2836_ic_reg_access,
  &bcm2835_cprman_reg_access,
  //&bcm2835_mailbox_reg_access,
};

/* Exclusive virtualization mmio list */
virt_excl_mmio_reg_access_t *virt_excl_devices[] = {
  &bcm2835_ic_reg_access,
  &gpio_reg_access,
  //&bcm2835_mailbox_reg_access,
};

void virt_mmio_reg_reset(void){
  int i;
  for(i=0; i< sizeof(virt_full_devices)/sizeof(virt_full_devices[0]); i++){
    if(virt_full_devices[i]->reg_reset_fn != NULL)
      virt_full_devices[i]->reg_reset_fn();
  }
  for(i=0; i< sizeof(virt_excl_devices)/sizeof(virt_excl_devices[0]); i++){
    if(virt_excl_devices[i]->reg_reset_fn != NULL)
      virt_excl_devices[i]->reg_reset_fn();
  }
}

int virt_mmio_reg_access(vcpu_t *vcpu,
       uint64_t opcode, phys_addr_t reg_addr, int rw){
  int i;
  virt_mmio_reg_read_fn_t   *reg_read_fn;
  virt_mmio_reg_write_fn_t  *reg_write_fn;

  log_debug("instruction code : %#x\n", opcode);
  log_debug("access addr : %#x\n", reg_addr);
  /* TODO : access size */
  log_debug("access size : %d\n", ((opcode>>30)&0b11)*16);


  for(i=0; i< sizeof(virt_full_devices)/sizeof(virt_full_devices[0]); i++){
    if((reg_addr>=virt_full_devices[i]->mem_start)&&(reg_addr<=virt_full_devices[i]->mem_end)){
      reg_read_fn = virt_full_devices[i]->reg_read_fn;
      reg_write_fn = virt_full_devices[i]->reg_write_fn;
      break;
    }
  }

  if(i==sizeof(virt_full_devices)/sizeof(virt_full_devices[0])){
    for(i=0; ; i++){
      if(i == sizeof(virt_excl_devices)/sizeof(virt_excl_devices[0])){
        // This reg_oofset is not virt device addr
        log_error("This address is not virt mmio device addr, addr : %#8x\n", reg_addr);
        return -1;
      }
      if((reg_addr>=virt_excl_devices[i]->mem_start)&&(reg_addr<=virt_excl_devices[i]->mem_end)){
        reg_read_fn = virt_excl_devices[i]->reg_read_fn;
        reg_write_fn = virt_excl_devices[i]->reg_write_fn;
        break;
      }
    }
  }

  /* 
   * emulata a load or store instruction
   * 
   * LDR : ss 111 0 xx 01 x
   * STR : ss 111 0 xx 00 x
   * 
   */
  uint64_t Rt = opcode&0b11111;

  if(rw){
    /*store ope*/
    if(((opcode>>22)&0xff)==0b11100000){
    } else if(((opcode>>22)&0xff)==0b11100100){
    }else{
      log_error("Not Supported instruction\n");
      return -1;
    }
    
    return reg_write_fn(vcpu, reg_addr, vcpu->reg.x[Rt], ((opcode>>30)&0b11)*16);

  } else {
    /*load ope*/
    if(((opcode>>22)&0xff)==0b11100001){
    } else if(((opcode>>22)&0xff)==0b11100101){
    }else{
      log_error("Not Supported instruction\n");
      return -1;
    }

    return reg_read_fn(vcpu, reg_addr, &vcpu->reg.x[Rt],((opcode>>30)&0b11)*16);
  }
}

void virt_mmio_reg_context_save(vcpu_t *vcpu){
  int i;
  for(i=0; i< sizeof(virt_full_devices)/sizeof(virt_full_devices[0]); i++){
    if(virt_full_devices[i]->reg_save_fn!=NULL)
      virt_full_devices[i]->reg_save_fn(vcpu);
  }
}

void virt_mmio_reg_context_restore(vcpu_t *vcpu){
  int i;
  for(i=0; i< sizeof(virt_full_devices)/sizeof(virt_full_devices[0]); i++){
    if(virt_full_devices[i]->reg_restore_fn!=NULL)
      virt_full_devices[i]->reg_restore_fn(vcpu);
  }
}

void virt_mmio_reg_assign(vm_t *vm){
  int i;
  for(i=0; i< sizeof(virt_excl_devices)/sizeof(virt_excl_devices[0]); i++){
    if(virt_excl_devices[i]->reg_assign_fn!=NULL)
      if(virt_excl_devices[i]->reg_assign_fn(vm) == -1){
        hyp_panic("Failed to assign virt mmio reg to vm %s\n",
            vm->name);
      }
  }
}

void virt_mmio_reg_release(vm_t *vm){
  int i;
  for(i=0; i< sizeof(virt_excl_devices)/sizeof(virt_excl_devices[0]); i++){
    if(virt_excl_devices[i]->reg_release_fn!=NULL)
      virt_excl_devices[i]->reg_release_fn(vm);
  }
}

/* Does not support 64bit read or write. */
int mmio_read(phys_addr_t addr, 
        void *dst , uint8_t size){
  
  switch(size){
    case 8:
      *(uint8_t *)dst = *(uint8_t *)addr;
      break;
    case 16:
      *(uint16_t *)dst = *(uint8_t *)addr;
      break;
    case 32:
      *(uint32_t *)dst = *(uint8_t *)addr;
      break;
    default:
      return -1;
  }
  return 0;
}

int mmio_write(phys_addr_t addr, 
        uint64_t value, uint8_t size){
  
  switch(size){
    case 8:
      *(uint8_t *)addr = (uint8_t)value;
      break;
    case 16:
      *(uint16_t *)addr = (uint16_t)value;
      break;
    case 32:
      *(uint32_t *)addr = (uint32_t)value;
      break;
    default:
      return -1;
  }
  return 0;
}
