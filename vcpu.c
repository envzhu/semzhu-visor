#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "hyp_mmu.h"
#include "pcpu.h"
#include "vm.h"
#include "vcpu.h"
#include "vtimer.h"
#include "virt_mmio.h"
#include "hyp_security.h"


static vcpu_t vcpus[VCPU_NUM];
static uint32_t num_vcpu = 0;

/*
static void _vcpu_init(vcpu_t *vcpu, uint32_t id, uint8_t vid, vm_t *vm){
    vcpu->id = id;
    vcpu->cpu_id = vid;
    vcpu->sysreg.pc = 0;
    vcpu->vm = vm;


    vcpu_reset(vcpu);
}*/

static vcpu_t *vcpu_alloc(void){
  if(num_vcpu >= VCPU_NUM)
      hyp_panic("There is no vcpu_t block!");

  memset(&vcpus[num_vcpu], 0, sizeof(vcpu_t));
  return &vcpus[num_vcpu++];
}

vcpu_t *vcpu_create(vm_t *vm, uint32_t vcpu_id, phys_addr_t vttbr, char *hyp_msg, phys_addr_t entry_addr){
  vcpu_t *vcpu = vcpu_alloc();
  
  vcpu->vm = vm;
  vcpu->vcpu_id = vcpu_id;
  vcpu->next = NULL;  
  vcpu->state = VCPU_STATE_INIT;
  vcpu->vttbr = vttbr;
  vcpu->hyp_msg = hyp_msg;
  log_info("hyp_msg addr : %#8x\n", vcpu->hyp_msg);

  vcpu->reg.x[0] = 0x800000;
  vcpu->sysreg.pc   = entry_addr;
  vcpu->sysreg.cpsr = CPSR_M_EL1h;
  vcpu->sysreg.sp_el0 = 0;
  vcpu->sysreg.sp_el1 = 0;
  vcpu->sysreg.vbar_el1 = 0;
  vcpu->sysreg.esr_el1  = 0;
  vcpu->sysreg.elr_el1  = 0;
  vcpu->sysreg.far_el1  = 0;
  vcpu->sysreg.spsr_el1 = 0;
  vcpu->sysreg.sctlr_el1= 0x00C50838;
  vcpu->sysreg.tcr_el1  = 0;
  vcpu->sysreg.ttbr0_el1  = 0;
  vcpu->sysreg.ttbr1_el1  = 0;
  vcpu->sysreg.midr_el1 = 0x410FD032;
  //vcpu->sysreg.mpidr_el1, vmpidr_el2);
  READ_SYSREG(vcpu->sysreg.mpidr_el1, vmpidr_el2);

  hyp_security_vcpu_init(vcpu);

  log_info("&vcpu : %#8x, &vcpu.reg : %#8x\n", vcpu, &vcpu->reg);
  return vcpu;
}

void vcpu_reset(vcpu_t *vcpu){
  memset(&vcpu->reg, 0, sizeof(vcpu_reg_t));
  memset(&vcpu->sysreg, 0, sizeof(vcpu_sysreg_t));
  
  vcpu_save_all_sysregs(vcpu);
  
  vcpu->state = VCPU_STATE_INIT;
  vcpu->sysreg.cpsr = CPSR_M_EL1h;
}

void vcpu_ready(vcpu_t *vcpu){
  if(vcpu->state == VCPU_STATE_RUN)
    hyp_panic("! this is already start !");

  vcpu->state = VCPU_STATE_READY;
  
  vcpu->vm->scheduler->scheduler_add(vcpu);
}

void vcpu_active(vcpu_t *vcpu, pcpu_t *phys_cpu){
  /*
  if(phys_cpu->current_vcpu != NULL)
      hyp_panic("There is already executing vcpu!");
    */
  if(vcpu == NULL)
    hyp_panic("! Cannot active NULL VCPU\n");
    
  if(vcpu->state == VCPU_STATE_RUN)
    hyp_panic("! This vcpu has already run !");

  vcpu->phys_cpu = phys_cpu;
  phys_cpu->current_vcpu = vcpu;

  vcpu->state = VCPU_STATE_RUN;
  
  log_info("Active vm:%s vcpu_id:%d\n", vcpu->vm->name, vcpu->vcpu_id);
}

void vcpu_off(vcpu_t *vcpu){
    if(vcpu->phys_cpu->current_vcpu == vcpu){
        vcpu->phys_cpu->current_vcpu = NULL;
    }
    if(vcpu->state == VCPU_STATE_READY){
      /* Remove vcpu from ready queue */
      vcpu->vm->scheduler->scheduler_remove(vcpu);
      
    }
    
    vcpu->state = VCPU_STATE_INIT;
}

/*
void vcpu_stop(void){
    exec_cpu = NULL;
}

void vcpu_save_state(vcpu_t *vcpu){
    if(vcpu==NULL){
        tv_abort("! Cannot save NULL VCPU\n");
        return;
    }
/*
    for(HW i=0;i<14;i++){
        vcpu->reg[i] = _sp->reg[i];
    }

    virtual_gic_save_state(vcpu->vm->vgic, vcpu->vid);*/
/*}

vcpu_t *vcpu_find_by_id(uint32_t id){
    return &(vcpus[id]);
}

vcpu_t *vcpu_get_last( void )
{
    return last_cpu;
}

void vcpu_wakeup(vcpu_t *vcpu){
    vcpu->state = VCPU_STATE_WAIT;
}

*/


void vcpu_sleep(vcpu_t *vcpu){
  
  if(vcpu->state != VCPU_STATE_RUN){
      hyp_panic("This vcpu is not running. So cannot sleep. vm:%s vcpu_id:%d\n",
          vcpu->vm->name, vcpu->vcpu_id);

      return;
  }
  
  log_info("Sleep vm:%s vcpu_id:%d\n", vcpu->vm->name, vcpu->vcpu_id);
  
  emulate_vtimer(vcpu);
  
  vcpu->vm->scheduler->scheduler_remove(vcpu);
    
  if(vcpu->phys_cpu->current_vcpu == vcpu)
    vcpu->phys_cpu->current_vcpu = NULL;
  vcpu->state = VCPU_STATE_SLEEP;
}

/*
void vcpu_make_wait(vcpu_t *vcpu){
    if(exec_cpu==vcpu){
        vcpu_save_state(vcpu);
        vcpu->state = VCPU_STATE_SLEEP;
        exec_cpu = NULL;
    } else {
        tv_abort("why wait!?\n");
    }
}


void vcpu_preemption(vcpu_t *vcpu){
    if(vcpu==NULL){
        if(exec_cpu==NULL && vcpu_get_last()){
            vm_stop(vcpu_get_last()->vm);
        } else {
            exec_cpu = NULL;
        }
        return;
    }
    if(exec_cpu==NULL) {
        vm_start(vcpu->vm);
        vcpu_start(vcpu);
        return;
    }
    if(exec_cpu==vcpu) return;
    if(last_cpu==vcpu){
        exec_cpu = vcpu;
        return;
    }
    vcpu_save_state(exec_cpu);
    if(exec_cpu->vm != vcpu->vm){
        vm_stop(exec_cpu->vm);
        vm_start(vcpu->vm);
    }
    exec_cpu->state = VCPU_STATE_WAIT;
    exec_cpu = NULL;
    vcpu_start(vcpu);
}


*/

phys_addr_t vcpu_get_pc_phys_addr(vcpu_t *vcpu){
  return el1va2pa(vcpu->sysreg.pc);
}

/* Virtual interrupt functions  */

/* Set the virtual SError and abort pending flag. */
void vcpu_do_vserror(vcpu_t *vcpu){
  vcpu->vic.vserror_pending |= 1;
  
  if(vcpu->state ==VCPU_STATE_SLEEP)
    vcpu_ready(vcpu);
}

/* Cause a virtual irq. */
void vcpu_do_virq(vcpu_t *vcpu){
  vcpu->vic.virq_pending |= 1;
  
  if(vcpu->state ==VCPU_STATE_SLEEP)
    vcpu_ready(vcpu);
  /* 
   * TODO:
   *  If vcpu->phys_cpu is not this phys_cpu, 
   *  wake up the physical cpu by mailbox.
   */
}

/* Set virtual fiq flag. */
void vcpu_do_vfiq(vcpu_t *vcpu){
  vcpu->vic.vfiq_pending |= 1;
  
  if(vcpu->state == VCPU_STATE_SLEEP)
    vcpu_ready(vcpu);
  /* 
   * TODO:
   *  If vcpu->phys_cpu is sleep, 
   *  wake up the physical cpu by mailbox.
   */
}

void set_vintr(vcpu_t *vcpu){
  uint64_t hcr_el2;

  READ_SYSREG(hcr_el2, HCR_EL2);
    hcr_el2 &= ~(HCR_VSE|HCR_VF|HCR_VI);

  if(vcpu->vic.vserror_pending){
    log_debug("Cause virtual SError\n");
    vcpu->vic.vserror_pending = 0;
    hcr_el2 |= HCR_VSE;

  } else if(vcpu->vic.vfiq_pending){
    log_debug("Cause virtual fiq\n");
    vcpu->vic.vfiq_pending = 0;
    hcr_el2 |= HCR_VF;

  } else if(vcpu->vic.virq_pending){
    log_debug("Cause virtual irq\n");
    vcpu->vic.virq_pending = 0;
    hcr_el2 |= HCR_VI;
  }

  WRITE_SYSREG(HCR_EL2, hcr_el2);
}

void vcpu_save_all_sysregs(vcpu_t *vcpu){

  READ_SYSREG(vcpu->sysreg.pc, elr_el2);
  READ_SYSREG(vcpu->sysreg.cpsr, spsr_el2);
  READ_SYSREG(vcpu->sysreg.sp_el0, sp_el0);
  READ_SYSREG(vcpu->sysreg.sp_el1, sp_el1);
  READ_SYSREG(vcpu->sysreg.vbar_el1, vbar_el1);
  READ_SYSREG(vcpu->sysreg.esr_el1, esr_el1);
  READ_SYSREG(vcpu->sysreg.elr_el1, elr_el1);
  READ_SYSREG(vcpu->sysreg.far_el1, far_el1);
  READ_SYSREG(vcpu->sysreg.par_el1, par_el1);
  READ_SYSREG(vcpu->sysreg.spsr_el1, spsr_el1);
  READ_SYSREG(vcpu->sysreg.sctlr_el1, sctlr_el1);
  READ_SYSREG(vcpu->sysreg.tcr_el1, tcr_el1);
  READ_SYSREG(vcpu->sysreg.ttbr0_el1, ttbr0_el1);
  READ_SYSREG(vcpu->sysreg.ttbr1_el1, ttbr1_el1);
  READ_SYSREG(vcpu->sysreg.mair_el1, mair_el1);
  READ_SYSREG(vcpu->sysreg.midr_el1, vpidr_el2);
  READ_SYSREG(vcpu->sysreg.mpidr_el1, vmpidr_el2);
  READ_SYSREG(vcpu->sysreg.cpacr_el1, cpacr_el1);

  READ_SYSREG(vcpu->sysreg.contextidr_el1, contextidr_el1);
  READ_SYSREG(vcpu->sysreg.tpidr_el0, tpidr_el0);
  READ_SYSREG(vcpu->sysreg.tpidr_el1, tpidr_el1);
  READ_SYSREG(vcpu->sysreg.tpidrro_el0, tpidrro_el0);


  /* Generic Timer */
  asm volatile("isb");
  READ_SYSREG(vcpu->sysreg.cntv_ctl_el0, CNTV_CTL_EL0);
  READ_SYSREG(vcpu->sysreg.cntv_cval_el0, CNTV_CVAL_EL0);
  READ_SYSREG(vcpu->sysreg.cntvoff_el2, CNTVOFF_EL2);

  READ_SYSREG(vcpu->sysreg.cntkctl_el1, CNTKCTL_EL1);
  READ_SYSREG(vcpu->sysreg.cntp_ctl_el0, CNTP_CTL_EL0);
  READ_SYSREG(vcpu->sysreg.cntp_cval_el0, CNTP_CVAL_EL0);
}

void vcpu_restore_all_sysregs(vcpu_t *vcpu){

  WRITE_SYSREG(elr_el2, vcpu->sysreg.pc);
  WRITE_SYSREG(spsr_el2, vcpu->sysreg.cpsr);
  WRITE_SYSREG(sp_el0, vcpu->sysreg.sp_el0);
  WRITE_SYSREG(sp_el1, vcpu->sysreg.sp_el1);
  WRITE_SYSREG(vbar_el1, vcpu->sysreg.vbar_el1);
  WRITE_SYSREG(esr_el1, vcpu->sysreg.esr_el1);
  WRITE_SYSREG(elr_el1, vcpu->sysreg.elr_el1);
  WRITE_SYSREG(far_el1, vcpu->sysreg.far_el1);
  WRITE_SYSREG(par_el1, vcpu->sysreg.par_el1);
  WRITE_SYSREG(spsr_el1, vcpu->sysreg.spsr_el1);
  WRITE_SYSREG(sctlr_el1, vcpu->sysreg.sctlr_el1);
  WRITE_SYSREG(tcr_el1, vcpu->sysreg.tcr_el1);
  WRITE_SYSREG(ttbr0_el1, vcpu->sysreg.ttbr0_el1);
  WRITE_SYSREG(ttbr1_el1, vcpu->sysreg.ttbr1_el1);
  WRITE_SYSREG(mair_el1, vcpu->sysreg.mair_el1);
  WRITE_SYSREG(vpidr_el2, vcpu->sysreg.midr_el1);
  WRITE_SYSREG(vmpidr_el2, vcpu->sysreg.mpidr_el1);
  WRITE_SYSREG(cpacr_el1, vcpu->sysreg.cpacr_el1);

  WRITE_SYSREG(contextidr_el1, vcpu->sysreg.contextidr_el1);  
  WRITE_SYSREG(tpidr_el0, vcpu->sysreg.tpidr_el0);
  WRITE_SYSREG(tpidr_el1, vcpu->sysreg.tpidr_el1);
  WRITE_SYSREG(tpidrro_el0, vcpu->sysreg.tpidrro_el0);

  /* Generic Timer */
  WRITE_SYSREG(CNTV_CTL_EL0, vcpu->sysreg.cntv_ctl_el0);
  WRITE_SYSREG(CNTV_CVAL_EL0, vcpu->sysreg.cntv_cval_el0);
  WRITE_SYSREG(CNTVOFF_EL2, vcpu->sysreg.cntvoff_el2);

  WRITE_SYSREG(CNTKCTL_EL1, vcpu->sysreg.cntkctl_el1);
  WRITE_SYSREG(CNTP_CTL_EL0, vcpu->sysreg.cntp_ctl_el0);
  WRITE_SYSREG(CNTP_CVAL_EL0, vcpu->sysreg.cntp_cval_el0);
  
  asm volatile("isb");
}

void vcpu_context_save(vcpu_t *vcpu, vcpu_reg_t *vcpu_reg){
  vcpu_freg_save(vcpu);
  vcpu_save_all_sysregs(vcpu);
  vtimer_context_save(vcpu);
  memcpy(&vcpu->reg, vcpu_reg, sizeof(vcpu_reg_t));
}

void vcpu_context_switch(vcpu_t *vcpu){
 if(vcpu == NULL)
    hyp_panic("You cannnot context switch to NULL vcpu.\n");

  /*
   * If current vcpu != last vcpu,
   * we need to switch registers for fpu, system registers and virt mmio registers,  too. 
   */
    
  if(vcpu->phys_cpu->last_vcpu != vcpu){
    log_info("Dynamic vcpu context switch\n");
    set_vttbr(vcpu->vttbr);
    vcpu_freg_restore(vcpu);
    vtimer_context_restore(vcpu);
    virt_mmio_reg_context_save(vcpu);
    vcpu_restore_all_sysregs(vcpu);
  }

  set_vintr(vcpu);
  dispatch(vcpu);
}

/* TODO */
void vcpu_context_dump(vcpu_t *vcpu, log_level_t level){
  int i;
  
  log_printf(level, "========= Dump vcpu context start =========\n");
  
  log_printf(level, "vcpu->vm->name : %s", vcpu->vm->name);
  log_printf(level, "vcpu->vcpu_id:  %d", vcpu->vcpu_id);
  
  for(i=0; i<31; i++)
    log_printf(level, "$X%d : %#8x", i, vcpu->reg.x[i]);
  /*
  for(i=0; i<32; i++)
    log_printf(level, "vcpu->reg.q[%d] : %#8x", i, vcpu->reg.q[0]);
  */

  log_printf(level, "PC   : %#8x", i, vcpu->sysreg.pc);
  log_printf(level, "CPSR : %#8x", i, vcpu->sysreg.cpsr);
  log_printf(level, "SP_EL0   : %#8x", i, vcpu->sysreg.sp_el0);
  log_printf(level, "SP_EL1   : %#8x", i, vcpu->sysreg.sp_el1);
  log_printf(level, "VBAR_EL1 : %#8x", i, vcpu->sysreg.vbar_el1);
  log_printf(level, "SPSR_EL1 : %#8x", i, vcpu->sysreg.spsr_el1);
  log_printf(level, "ELR_EL1  : %#8x", i, vcpu->sysreg.elr_el1);
  log_printf(level, "ESR_EL1 : %#8x", i, vcpu->sysreg.esr_el1);
  log_printf(level, "FAR_EL1  : %#8x", i, vcpu->sysreg.far_el1);
  log_printf(level, "PAR_EL1  : %#8x", i, vcpu->sysreg.par_el1);
  log_printf(level, "SCTLR_EL1  : %#8x", i, vcpu->sysreg.sctlr_el1);

  log_printf(level, "=================   End   =================\n");
}

vcpu_t *set_cur_vcpu(uint64_t cpu_id, vcpu_t *vcpu){
  
  if(cpu_id >= CPU_NUM){
    hyp_panic("Illegal cpu id!");
  }

  vcpu->phys_cpu->current_vcpu = vcpu;
}
