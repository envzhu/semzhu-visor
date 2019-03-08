#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "hyp_call.h"
#include "hyp_security.h"
#include "hyp_timer.h"
#include "coproc_def.h"
#include "virt_mmio.h"
#include "pcpu.h"
#include "vcpu.h"
#include "virq.h"

void vm_interrupt_handler(pcpu_t *phys_cpu, uint64_t vec_num, uint32_t esr);

static void data_abort_print(log_level_t level, uint32_t iss){
  uint64_t far_el2;
  uint64_t hpfar_el2;

  switch(iss&0b111100){
    case 0b000000:
      log_printf(level, "Address size fault\n");
      break;
    case 0b000100:
      log_printf(level, "Translation fault\n");
      break;
    case 0b001000:
      log_printf(level, "Access flag fault\n");
      break;
    case 0b01100:
      log_printf(level, "Permission fault\n");
      break;
    case 0b10000:
      log_printf(level, "Synchronous External abort, not on translation table walk\n");
      break;
    case 0b10100:
      log_printf(level, "Synchronous External abort, on translation table walk\n");
      break;
    case 0b11000:
      log_printf(level, "Synchronous parity or ECC error on memory access, not on translation table walk\n");
      break;
    case 0b11100:
      log_printf(level, "Synchronous parity or ECC error on memory access on translation table walk\n");
      break;
    case 0b100000:
      log_printf(level, "Alignment fault\n");
      break;
    default:
      log_printf(level, "unknown iss\n");
      break;
  }
  
  
  if(iss&(1<<6))
    log_printf(level, "Abort caused by an instruction writing to a memory location.\n");
  else
    log_printf(level, "Abort caused by an instruction reading from a memory location.\n");
  
  if(iss&(1<<7))
    log_printf(level, "Fault on the stage 2 translation of an access for a stage 1 translation table walk.\n");
  else
    log_printf(level, "Fault not on a stage 2 translation for a stage 1 translation table walk.\n");

  if(iss&(1<<10))
    log_printf(level, "far_elx is not valid\n");
  else{
    READ_SYSREG(far_el2, FAR_EL2);
    READ_SYSREG(hpfar_el2, HPFAR_EL2);

    log_printf(level, "Fault va : %#8x, iss : %#x\n", far_el2, iss);
    log_printf(level, "Fault ipa : %#8x\n", ((hpfar_el2&0xfffffff8) << 8) + (far_el2&0xfff));
  }
}

void vm_interrupt_entry(uint64_t vec_num, uint32_t esr){
  
  pcpu_t *phys_cpu = get_current_phys_cpu();
  
  /* Set last vcpu */
  phys_cpu->last_vcpu = phys_cpu->current_vcpu;

  /* 
   * We have already saved general purpose registers in the interrupt vector.
   * So we need to save other registers if we switch to another guest OS.
   */
  vcpu_freg_save(phys_cpu->current_vcpu);
  vcpu_save_all_sysregs(phys_cpu->current_vcpu);
  
  if(phys_cpu->current_vcpu->security.error == 0)
    hyp_security_check(phys_cpu->current_vcpu, esr);

  if(phys_cpu->current_vcpu->security.error == 0)
    vm_interrupt_handler(phys_cpu, vec_num, esr);
  else
    hyp_security_error(phys_cpu->current_vcpu);

  do_schedule(phys_cpu);

  if(phys_cpu->current_vcpu == NULL){
    virt_mmio_reg_reset();
    INTR_ENABLE;
    while(1)
      asm volatile("wfe");
  }

  vcpu_context_switch(phys_cpu->current_vcpu);
}


void vm_interrupt_handler(pcpu_t *phys_cpu, uint64_t vec_num, uint32_t esr){ 
  uint8_t ec = esr >> 26;
  uint8_t il = (esr >> 25) & 1;
  uint16_t iss = esr & 0x1FFF;
  vcpu_t *cur_vcpu = phys_cpu->current_vcpu;
  phys_addr_t far_el2;
  phys_addr_t hpfar_el2;
  phys_addr_t data_pa;
  uint64_t hcr_el2;
  
  
  phys_addr_t inst_ipa = el1va2ipa(cur_vcpu->sysreg.pc);
  phys_addr_t inst_pa = el1va2pa(cur_vcpu->sysreg.pc);
  
  log_debug("vec_num : %#x \n", vec_num);
  log_debug("Interrupt has caused in physical cpu id %d\nEC:%#2x\nCPSR : %#4x, ELR_EL2:%#8x\n",
      phys_cpu->cpu_id, ec, cur_vcpu->sysreg.cpsr, cur_vcpu->sysreg.pc);

  log_debug("IPA of cause instruction : %#8x\n", inst_ipa);
  log_debug("PA of cause instruction : %#8x\n", inst_pa);

  switch(ec){
    case 0x00:
      
      // Unknown reason
      //if(extract(*(uint32_t *)inst_pa, 20, 31)==0xd40){
        log_debug("HVC from EL0\n");
        hyp_call(phys_cpu->current_vcpu, extract(*(phys_addr_t *)inst_pa, 5, 20));
        cur_vcpu->sysreg.pc += 4;
        
        WRITE_SYSREG(ELR_EL2, cur_vcpu->sysreg.pc);
        
        break;
      //}
      #if 0
      log_debug("instruction code : %#8x\n",extract(*(uint32_t *)inst_pa, 20, 31));
      hyp_panic("Unknown Reason exception!\n");
      vcpu_do_vserror(cur_vcpu);
      break;
      #endif
    case 0x01:
      // WFI or WFE instruction execution
      log_debug("WFI or WFE was executed!\n");
      cur_vcpu->sysreg.pc += 4;
      vcpu_sleep(cur_vcpu);
      break;

    case 0x03:
      // MCR or MRC access to CP15 a that is not reported using EC 0x00
    case 0x04:
      // MCRR or MRRC access to CP15 a that is not reported using EC 0x00
    case 0x05:
      // MCR or MRC access to CP14
    case 0x06:
      // LDC or STC access to CP14
    case 0x07:
      // Access to SIMD or floating-point registers, 
      // excluding (HCR_EL2.TGE==1) traps
    case 0x08:
      // MCR or MRC access to CP10 that is not reported using EC 0x07 .
      // This applies only to ID Group traps
    case 0x0C:
      // MRRC access to CP14
    case 0x0E:
      // Illegal Execution State
      break;
    case 0x15:
      // SVC instruction execution from AArch64
      log_debug("SVC\n");
      break;

    case 0x16:
      // HVC instruction execution, when HVC is not disabled from AAech64
      hyp_call(cur_vcpu, iss);
      break;

    case 0x17:
      // SMC instruction execution, when SMC is not disabled from AAech64
      break;

    case 0x20:
      // Instruction Abort from a lower Exception level
      hyp_panic(" Instruction Abort from a lower Exception level\nISS : %#8x\nFault on the Stage %x translation\n", iss, iss&0x80+1);

      /* iss
       * bit
       * |24---~---10|9 |8|7    |6|5----0|
       * |RES0       |EA|0|SiPTW|0| IFSC |
       */
      switch(iss&0x3C){
        case 0x0:
          hyp_panic("Address Size fault at EL%x\n", iss&0b11);
        case 0x2:
          hyp_panic("Translation fault at EL%x\n", iss&0b11);
        case 0x4:
          hyp_panic("Access Flag fault at EL%x\n", iss&0b11);
        case 0xC:
          hyp_panic("Permission fault at EL%x\n", iss&0b11);
        case 0x10:
          hyp_panic("Synchronous External abort\n");
        case 0x18:
          hyp_panic("Synchronous Parity error on a memory access\n");
        case 0x14:
          hyp_panic("Synchronous External abort on a translation table walk at EL%x\n", iss&0b11);      
        case 0x1C:
          hyp_panic("Synchronous Parity error on a memory access on a translation table walk at EL%x\n", iss&0b11);
        case 0x20:
          hyp_panic("Alignment fault\n");
        case 0x30:
          hyp_panic("TLB Conflict fault\n");
      }
      break;

    case 0x22:
      // Misaligned PC exception
      break;
    case 0x24:
      // Data Abort from a lower Exception level
      
      if(iss&(1<<10)){
        log_debug("far_elx is not valid\n");
      
      }else{
        // if far_el2 is valid 

        READ_SYSREG(far_el2, FAR_EL2);
        READ_SYSREG(hpfar_el2, HPFAR_EL2);
        data_pa = ((hpfar_el2&0xfffffff8) << 8) + (far_el2&0xfff);

        if(virt_mmio_reg_access(cur_vcpu, 
            *(uint32_t *)inst_pa, data_pa, iss&(1<<6)) < 0){
        //hyp_panic("Data Abort from EL0 or EL1\n");
        data_abort_print(LOG_DEBUG, iss);
        vcpu_do_vserror(cur_vcpu);
        }else{
          cur_vcpu->sysreg.pc += 4;
          WRITE_SYSREG(ELR_EL2, cur_vcpu->sysreg.pc);
        }
      }
      break;
    
    case 0x26:
      // Stack Pointer Alignment exception
    case 0x2C:
      // Floating-point exception, if supported from AArch64
    case 0x2F:
      // SError interrupt
    case 0x30:
      // Breakpoint exception from a lower Exception level
    case 0x32:
      // Software Step exception from a lower Exception level
    case 0x34:
      // Watchpoint exception from a lower Exception level
    case 0x3C:
      // BRK instruction execution in AArch64 state
      // This is reported in ESR_EL3 only if a BRK instruction is executed at EL3.
    default:
      log_info("Virtual serror!\n");
      vcpu_do_vserror(cur_vcpu);
      break; 
  }
}

void vm_irq_interrupt_entry(uint64_t vec_num, uint32_t esr){ 
  pcpu_t *phys_cpu = get_current_phys_cpu();
  phys_addr_t elr_el2;

  /* Set last vcpu */
  phys_cpu->last_vcpu = phys_cpu->current_vcpu;
  
  READ_SYSREG(elr_el2, ELR_EL2);
  log_debug("IRQ Interrupt has caused from VM in physical cpu id %d\nELR_EL2:%#8x\n",
      phys_cpu->cpu_id, elr_el2);
  /* 
  * We have already saved general purpose registers in the interrupt vector.
  * So we need to save other registers if we switch to another guest OS.
  */
  vcpu_freg_save(phys_cpu->current_vcpu);
  vcpu_save_all_sysregs(phys_cpu->current_vcpu);

  //log_debug("Physical address of cause instruction : %#8x\n", inst_pa);
  if(hyp_timer_irq_is_pending(phys_cpu->cpu_id))
    hyp_timer_intr(phys_cpu);
  else
    virt_intr_handler(phys_cpu->current_vcpu);
  
  do_schedule(phys_cpu);

  if(phys_cpu->current_vcpu == NULL){
    virt_mmio_reg_reset();
    cpu_relax();
  }

  vcpu_context_switch(phys_cpu->current_vcpu);
}

void hyp_irq_interrupt_handler(uint64_t vec_num, uint32_t esr){ 
  pcpu_t *phys_cpu = get_current_phys_cpu();
  phys_addr_t elr_el2;

  /* Set last vcpu */
  phys_cpu->last_vcpu = phys_cpu->current_vcpu;
  
  READ_SYSREG(elr_el2, ELR_EL2);
  log_debug("IRQ Interrupt to hypervisor has caused in physical cpu id :%d, ELR_EL2:%#8x\n",
      phys_cpu->cpu_id, elr_el2);
  
  if(hyp_timer_irq_is_pending(phys_cpu->cpu_id))
    hyp_timer_intr(phys_cpu);
  else
    virt_intr_handler(phys_cpu->last_vcpu);
      
  do_schedule(phys_cpu);
  if(phys_cpu->current_vcpu == NULL){
    virt_mmio_reg_reset();
    INTR_ENABLE;
    while(1)
      asm volatile("wfe");
  }

  vcpu_context_switch(phys_cpu->current_vcpu);
}

void hyp_error_interrupt_handler(uint64_t vec_num, uint32_t esr){ 
  uint32_t ec = esr >> 26;
  uint32_t il = (esr >> 25) & 1;
  uint32_t iss = esr & 0x1FFF;
  pcpu_t *phys_cpu = get_current_phys_cpu();
  phys_addr_t elr_el2;

  READ_SYSREG(elr_el2, ELR_EL2);

  log_error("vec_num : %#x \n", vec_num);
  log_error("Interrupt from hyp mode has caused in physical cpu id %d\nELR_EL2:%#8x\n",
      phys_cpu->cpu_id, elr_el2);
  
  switch(ec){
    case 0x00:
      // Unknown reason
      log_crit("Unknown Reason exception!\n");
      break;
    case 0x0C:
      // MRRC access to CP14
    case 0x0E:
      // Illegal Execution State
    case 0x15:
      // SVC instruction execution from AArch64
      break;
    case 0x16:
      // HVC instruction execution, when HVC is not disabled from AAech64
      break;
    case 0x17:
      // SMC instruction execution, when SMC is not disabled from AAech64
      break;
    case 0x21:
      // Instruction Abort taken without a change in Exception level
      break;
    case 0x22:
      // Misaligned PC exception
      break;
    case 0x25:
      // Data Abort taken without a change in Exception level
      data_abort_print(LOG_CRIT, iss);
      log_crit("Data Abort from EL2\n");
      break;
    case 0x26:
      // Stack Pointer Alignment exception
    case 0x2C:
      // Floating-point exception, if supported from AArch64
    case 0x2F:
      // SError interrupt
    case 0x31:
      // Breakpoint exception taken without a change in Exception level
    case 0x33:
      // Software Step exception taken without a change in Exception level
    case 0x35:
      // Watchpoint exception taken without a change in Exception level
    case 0x3C:
      // BRK instruction execution in AArch64 state
      // This is reported in ESR_EL3 only if a BRK instruction is executed at EL3.
    default:
      cpu_hlt();
      break; 
  }
 hyp_panic("An error exception was taken in Hypervisor\n");
}

void aarch32_interrupt_handler(uint64_t vec_num, uint32_t esr){ 
  pcpu_t *phys_cpu = get_current_phys_cpu();
  phys_addr_t elr_el2;

  READ_SYSREG(elr_el2, ELR_EL2);

  log_debug("vec_num : %#x \n", vec_num);
  log_debug("Interrupt from AArch32 has caused in physical cpu id %d\nELR_EL2:%#8x\n", phys_cpu->cpu_id, elr_el2);
  hyp_panic("Does not Support AArch32 execution.\n");
}
