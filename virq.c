/* 
 * virtual_irq.c
 * IRQ virtualization
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "asm_func.h"
#include "vcpu.h"
#include "virq.h"

/*
 * BCM2837 Interrupt Controllers
 * 
 * BCM2837 has two Interrupt Controllers :
 *    1. BCM2835 Interrupt Controller
 * *      - Control both of arm core irqs and gpu irqs.
 *    2. BCM2836 Interrupt Controller
 *  *     - Control only arm core irqs such as pmu irq.
 * 
 * Orevisor does not allowed to use basic interrupt in BCM2836 Interrupt Controller.
 */


/* Todo : Support vector No. 64~71 interrupt in BCM2835 Interrupt Controller */

/* BCM2835 Interrupt Controller registers */
#define ARM_IC_BASE		0x3F00B000

#define ARM_IC_BASIC_IRQ_PENDING  (ARM_IC_BASE + 0x200)
#define ARM_IC_IRQ_PENDING_1	    (ARM_IC_BASE + 0x204)
#define ARM_IC_IRQ_PENDING_2	    (ARM_IC_BASE + 0x208)
#define ARM_IC_FIQ_CONTROL	      (ARM_IC_BASE + 0x20C)
#define ARM_IC_ENABLE_IRQ_1	      (ARM_IC_BASE + 0x210)
#define ARM_IC_ENABLE_IRQ_2	      (ARM_IC_BASE + 0x214)
#define ARM_IC_ENABLE_BASIC_IRQ   (ARM_IC_BASE + 0x218)
#define ARM_IC_DISABLE_IRQ_1	    (ARM_IC_BASE + 0x21C)
#define ARM_IC_DISABLE_IRQ_2	    (ARM_IC_BASE + 0x220)
#define ARM_IC_DISABLE_BASIC_IRQ  (ARM_IC_BASE + 0x224)



/* BCM2836 Interrupt Controller registers */
#define ARM_CORE_BASE			0x40000000

#define ARM_CORE_IRQ_CNTPSIRQ   0
#define ARM_CORE_IRQ_CNTPNSIRQ  1
#define ARM_CORE_IRQ_CNTHPIRQ   2
#define ARM_CORE_IRQ_CNTVIRQ    3
#define ARM_CORE_IRQ_MAILBOX0   4
#define ARM_CORE_IRQ_MAILBOX1   5
#define ARM_CORE_IRQ_MAILBOX2   6
#define ARM_CORE_IRQ_MAILBOX3   7
#define ARM_CORE_IRQ_GPU        8
#define ARM_CORE_IRQ_PMU        9
#define ARM_CORE_IRQ_AXI        10
#define ARM_CORE_IRQ_LOCAL_TIMER      11
#define ARM_CORE_IRQ_MAX        CORE_IRQ_TIMER)

#define ARM_CORE_CORE_TIMER_CONTROL    (ARM_CORE_BASE + 0x0)
#define ARM_CORE_CORE_TIMER_PRESCALER  (ARM_CORE_BASE + 0x8)
#define ARM_CORE_GPU_IRQ_ROUTING	(ARM_CORE_BASE + 0xC)
#define ARM_CORE_PMU_ROUTING_SET	(ARM_CORE_BASE + 0x10)
#define ARM_CORE_PMU_ROUTING_CLR	(ARM_CORE_BASE + 0x14)
#define ARM_CORE_CORE_TIMER_LS		(ARM_CORE_BASE + 0x1C)
#define ARM_CORE_CORE_TIMER_MS	  (ARM_CORE_BASE + 0x20)
#define ARM_CORE_LOCAL_TIMER_IRQ_ROUTING  (ARM_CORE_BASE + 0x24)
#define ARM_CORE_AXI_COUNT		(ARM_CORE_BASE + 0x2C)
#define ARM_CORE_AXI_IRQ		  (ARM_CORE_BASE + 0x30)
#define ARM_CORE_LOCAL_TIMER_CONTROL  (ARM_CORE_BASE + 0x34)
#define ARM_CORE_LOCAL_TIMER_WRITE    (ARM_CORE_BASE + 0x38)

#define ARM_CORE0_CORE_TIMER_INT_CONTROL  (ARM_CORE_BASE + 0x40)
#define ARM_CORE1_CORE_TIMER_INT_CONTROL  (ARM_CORE_BASE + 0x44)
#define ARM_CORE2_CORE_TIMER_INT_CONTROL  (ARM_CORE_BASE + 0x48)
#define ARM_CORE3_CORE_TIMER_INT_CONTROL  (ARM_CORE_BASE + 0x4C)

#define ARM_CORE0_MAILBOX_INT_CONTROL	(ARM_CORE_BASE + 0x50)
#define ARM_CORE1_MAILBOX_INT_CONTROL (ARM_CORE_BASE + 0x54)
#define ARM_CORE2_MAILBOX_INT_CONTROL	(ARM_CORE_BASE + 0x58)
#define ARM_CORE3_MAILBOX_INT_CONTROL	(ARM_CORE_BASE + 0x5C)

#define ARM_CORE0_IRQ_PENDING		(ARM_CORE_BASE + 0x60)
#define ARM_CORE1_IRQ_PENDING	  (ARM_CORE_BASE + 0x64)
#define ARM_CORE2_IRQ_PENDING		(ARM_CORE_BASE + 0x68)
#define ARM_CORE3_IRQ_PENDING		(ARM_CORE_BASE + 0x6C)

#define ARM_CORE0_FIQ_PENDING		(ARM_CORE_BASE + 0x70)
#define ARM_CORE1_FIQ_PENDING		(ARM_CORE_BASE + 0x74)
#define ARM_CORE2_FIQ_PENDING		(ARM_CORE_BASE + 0x78)
#define ARM_CORE3_FIQ_PENDING		(ARM_CORE_BASE + 0x7C)

vm_t *irq_vec_table[GPU_INTERRUPT_NUM];

uint32_t gpu_irq_is_pending(int nirq){
  int x =0;
  if(nirq < 32){
    return ((*(volatile uint32_t *)ARM_IC_IRQ_PENDING_1) & (1 << nirq)) ? 1: 0;
  } else if(nirq < 64){
    return ((*(volatile uint32_t *)ARM_IC_IRQ_PENDING_2) & (1 << (nirq-32)))? 1 : 0; 
  }
}

uint32_t gpu_irq_is_enable(int nirq){
  if(nirq < 32)
    return ((*(volatile uint32_t *)ARM_IC_DISABLE_IRQ_1)&(1 << nirq))? 1 : 0;
  else if(nirq < 64)
    return ((*(volatile uint32_t *)ARM_IC_DISABLE_IRQ_2)&(1 << (nirq-32)))? 1 : 0; 
}

uint32_t gpu_irq_set_enable(int nirq){
  if(nirq < 32)
    *(volatile uint32_t *)ARM_IC_ENABLE_IRQ_1 |= 1<<nirq;
  else if(nirq < 64)
    *(volatile uint32_t *)ARM_IC_ENABLE_IRQ_2 |= 1<<(nirq%32);  
}

void gpu_irq_set_disable(int nirq){
  if(nirq < 32)
    *(volatile uint32_t *)ARM_IC_DISABLE_IRQ_1 &= 1<<nirq;
  else if(nirq < 64)
    *(volatile uint32_t *)ARM_IC_DISABLE_IRQ_2 &= 1<<(nirq-32);
}

uint32_t vcpu_core_irq_is_pending(vcpu_t *vcpu){
  return (*(volatile uint32_t *)(ARM_CORE0_IRQ_PENDING+vcpu->phys_cpu->cpu_id*4 ))
            &~(1<<ARM_CORE_IRQ_CNTHPIRQ);
}

uint32_t vcpu_core_fiq_is_pending(vcpu_t *vcpu){
  return (*(volatile uint32_t *)(ARM_CORE0_FIQ_PENDING+vcpu->phys_cpu->cpu_id*4 ))
            &~(1<<ARM_CORE_IRQ_CNTHPIRQ);
}

uint32_t hyp_timer_irq_is_pending(uint32_t cpu_id){
  return ((*(volatile uint32_t *)(ARM_CORE0_IRQ_PENDING+cpu_id*4 ))
            &(1<<ARM_CORE_IRQ_CNTHPIRQ))?1 :0;
}

uint32_t hyp_timer_fiq_is_pending(uint32_t cpu_id){
  return ((*(volatile uint32_t *)(ARM_CORE0_FIQ_PENDING+cpu_id*4 ))
            &(1<<ARM_CORE_IRQ_CNTHPIRQ))?1 :0;
}

/*
 * BCM2835 interrupt contraller virtualization
 * In this hypervisor virtualize only gpu irq.
 * Do not support basic irq.
 */
static virt_mmio_reg_read_fn_t  bcm2835_ic_reg_read;
static virt_mmio_reg_write_fn_t bcm2835_ic_reg_write;
static virt_mmio_reg_context_save_fn_t    bcm2835_ic_reg_save;
static virt_mmio_reg_context_restore_fn_t   bcm2835_ic_reg_restore;
static virt_mmio_reg_reset_fn_t bcm2835_ic_reg_reset;

virt_excl_mmio_reg_access_t bcm2835_ic_reg_access = {
  0x3F00B000, 0x3F00BFFF,
  NULL,
  bcm2835_ic_reg_read,
  bcm2835_ic_reg_write,
  NULL,
  NULL,
};

static int bcm2835_ic_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){

  if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    case ARM_IC_BASIC_IRQ_PENDING:
      //if(vcpu->vm->vic.assigned_gpu_irq&VIRT_INTR_UART)
        *(uint32_t *)dst = *(volatile uint32_t *)addr;
      break;

    case ARM_IC_ENABLE_BASIC_IRQ :
    case ARM_IC_DISABLE_BASIC_IRQ:
      log_error("Do not access to this register\n");
      return -1;
      break;
  
    case ARM_IC_FIQ_CONTROL :
      *(uint32_t *)dst = vcpu->vm->vic.fiq_index;
      break;
    
    
    case ARM_IC_IRQ_PENDING_1:
    case ARM_IC_ENABLE_IRQ_1 :
    case ARM_IC_DISABLE_IRQ_1:
      *(uint32_t *)dst = *(uint32_t *)(addr&vcpu->vm->vic.assigned_gpu_irq);
      
    case ARM_IC_IRQ_PENDING_2:
    case ARM_IC_ENABLE_IRQ_2 :
    case ARM_IC_DISABLE_IRQ_2:
      *(uint32_t *)dst = *(volatile  uint32_t *)(addr&(vcpu->vm->vic.assigned_gpu_irq>>32));
    
    default:
      log_debug("Illegal access to unavailable address\n");
      return -1;
  }

  return 0;
}

static int bcm2835_ic_reg_write(vcpu_t *vcpu, 
              phys_addr_t addr, uint64_t value, uint8_t size){

  if(size != 32){
    log_error("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    case ARM_IC_BASIC_IRQ_PENDING:
    case ARM_IC_IRQ_PENDING_1:
    case ARM_IC_IRQ_PENDING_2:
      /*
       * These registers are read-only.
       * SO we return -1.
       */
      log_error("You cannot write to IRQ pending resisters.\n");
      return -1;
      break;

    case ARM_IC_ENABLE_BASIC_IRQ :
    case ARM_IC_DISABLE_BASIC_IRQ:
      log_error("Do not access to this register\n");
      return -1;
      break;

    case ARM_IC_FIQ_CONTROL :
      vcpu->vm->vic.fiq_index = value;
      break;
    
    case ARM_IC_ENABLE_IRQ_1 :
    case ARM_IC_DISABLE_IRQ_1:
      *(uint32_t *) addr = (uint32_t)(value & vcpu->vm->vic.assigned_gpu_irq);
      break;

    case ARM_IC_ENABLE_IRQ_2 :
    case ARM_IC_DISABLE_IRQ_2:
      *(uint32_t *)addr = (uint32_t)(value & vcpu->vm->vic.assigned_gpu_irq>>32);
      break;
      
    default:
      log_error("Illegal access to unavailable address\n");
      return -1;
  }

  return 0;
}

/* TODO */
static void bcm2835_ic_reg_release(vcpu_t *vcpu){
  /*
  * Release GPU Interrupts which are assigned to this vcpu 
  * when the cpu is released
  */
  *(volatile  uint32_t *)ARM_IC_DISABLE_IRQ_1 &= ~(uint32_t)vcpu->vm->vic.assigned_gpu_irq;
  *(volatile  uint32_t *)ARM_IC_DISABLE_IRQ_2 &= ~(uint32_t)(vcpu->vm->vic.assigned_gpu_irq>>32);
}

/*
 * BCM2836 interrupt contraller virtualization
 */
static virt_mmio_reg_read_fn_t  bcm2836_ic_reg_read;
static virt_mmio_reg_write_fn_t bcm2836_ic_reg_write;
static virt_mmio_reg_context_save_fn_t    bcm2836_ic_reg_save;
static virt_mmio_reg_context_restore_fn_t   bcm2836_ic_reg_restore;
static virt_mmio_reg_reset_fn_t bcm2836_ic_reg_reset;

virt_full_mmio_reg_access_t bcm2836_ic_reg_access = {
  0x40000000,0x4000007F,
  bcm2836_ic_reg_reset,
  bcm2836_ic_reg_read,
  bcm2836_ic_reg_write,
  bcm2836_ic_reg_save,
  bcm2836_ic_reg_restore,
};

static int bcm2836_ic_reg_read(vcpu_t *vcpu, 
              phys_addr_t addr, void *dst, uint8_t size){

  vcpu_t *src_vcpu;

  if(size != 32){
    log_debug("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    case ARM_CORE_CORE_TIMER_CONTROL :
    case ARM_CORE_CORE_TIMER_PRESCALER :
    case ARM_CORE_CORE_TIMER_LS :
    case ARM_CORE_CORE_TIMER_MS :
      *(uint32_t *)dst = *(uint32_t *)addr;
      break;

    case ARM_CORE_GPU_IRQ_ROUTING :
      *(uint32_t *)dst = vcpu->vm->vic.gpu_irq_route;
      break;
    
    case ARM_CORE_PMU_ROUTING_SET	 :
    case ARM_CORE_PMU_ROUTING_CLR	 :    

    case ARM_CORE_LOCAL_TIMER_IRQ_ROUTING :
    case ARM_CORE_AXI_COUNT :
    case ARM_CORE_AXI_IRQ :
    case ARM_CORE_LOCAL_TIMER_CONTROL :
    case ARM_CORE_LOCAL_TIMER_WRITE :
      log_debug("Do not read this register\n");
      return -1;
      break;

    case ARM_CORE0_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE1_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE2_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE3_CORE_TIMER_INT_CONTROL	:
      if( ((addr&0xf)/4) >= vcpu->vm->vcpu_num ){
        log_debug("You cannot access registers of the cpu which is not assigned to the vm.\n");
        return -1;
      }

      src_vcpu = vcpu->vm->vcpu[(addr&0xf)/4];
      *(uint32_t *) dst = src_vcpu->vic.core_timer_intr_enable;
      break; 

    case ARM_CORE0_MAILBOX_INT_CONTROL :
    case ARM_CORE1_MAILBOX_INT_CONTROL :
    case ARM_CORE2_MAILBOX_INT_CONTROL :
    case ARM_CORE3_MAILBOX_INT_CONTROL :
      if( ((addr&0xf)/4) >= vcpu->vm->vcpu_num ){
        log_debug("You cannot access registers of the cpu which is not assigned to the vm.\n");
        return -1;
      }

      src_vcpu = vcpu->vm->vcpu[(addr&0xf)/4];
      *(uint32_t *) dst = src_vcpu->vic.core_mbox_intr_enable;
      break; 
    
    case ARM_CORE0_IRQ_PENDING :
    case ARM_CORE1_IRQ_PENDING :
    case ARM_CORE2_IRQ_PENDING :
    case ARM_CORE3_IRQ_PENDING :

    case ARM_CORE0_FIQ_PENDING :
    case ARM_CORE1_FIQ_PENDING :
    case ARM_CORE2_FIQ_PENDING :
    case ARM_CORE3_FIQ_PENDING :
      /* Todo : Support FIQ */
      /* Todo : Hyp Timer and mail box in INTR pending*/
      /* TODO : see qemu */
      if( ((addr&0xf)/4) >= vcpu->vm->vcpu_num ){
        log_debug("You cannot access registers of the cpu which is not assigned to the vm.\n");
        return -1;
      }

      src_vcpu = vcpu->vm->vcpu[(addr&0xf)/4];
      if(src_vcpu->state == VCPU_STATE_RUN){
        *(uint32_t *)dst = *(volatile uint32_t *)(src_vcpu->phys_cpu->cpu_id*4 
            + addr&0xfffffff0);
      } else {
        /* TODO */
      }
      break;

    default:
      log_debug("Illegal access to unavailable address\n");
        vm_force_shutdown(vcpu->vm);
  }
}

static int bcm2836_ic_reg_write(vcpu_t *vcpu,
              phys_addr_t addr, uint64_t value, uint8_t size){
  
  vcpu_t *src_vcpu;

  if(size != 32){
    log_debug("Support read or write mmio only in 32bit.\n");
    return -1;
  }

  switch(addr){
    /* Do not allowed to write these register. */
    case ARM_CORE_CORE_TIMER_CONTROL :
    case ARM_CORE_CORE_TIMER_PRESCALER :
    case ARM_CORE_CORE_TIMER_LS :
    case ARM_CORE_CORE_TIMER_MS :
      *(uint32_t *) addr = value;
      break;

    case ARM_CORE_PMU_ROUTING_SET	 :
    case ARM_CORE_PMU_ROUTING_CLR	 :

    case ARM_CORE_LOCAL_TIMER_IRQ_ROUTING :
    case ARM_CORE_AXI_COUNT :
    case ARM_CORE_AXI_IRQ :
    case ARM_CORE_LOCAL_TIMER_CONTROL :
    case ARM_CORE_LOCAL_TIMER_WRITE :
      log_debug("Do not write this register\n");
      return -1;
      break;
  
    case ARM_CORE_GPU_IRQ_ROUTING :
      if(((value&0b11) < vcpu->vm->vcpu_num)
            && (((value&0b1100)>>2) < vcpu->vm->vcpu_num)){
        vcpu->vm->vic.gpu_irq_route = value;
      }else{
        log_debug("You cannot route gpu irq to the vcpu which is not assigned to this vm.\n");
        return -1;
      }
      break;
  
    case ARM_CORE0_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE1_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE2_CORE_TIMER_INT_CONTROL	:
    case ARM_CORE3_CORE_TIMER_INT_CONTROL	:
      if( ((addr&0xf)/4) >= vcpu->vm->vcpu_num ){
        log_debug("You cannot access registers of the cpu which is not assigned to the vm.\n");
        return -1;
      }

      src_vcpu = vcpu->vm->vcpu[(addr&0xf)/4];
      if(src_vcpu->state == VCPU_STATE_RUN){
        *(volatile uint32_t *)((src_vcpu->phys_cpu->cpu_id*4) 
            + addr&0xfffffff0) |= value & 0b10111011;
      }
      src_vcpu->vic.core_timer_intr_enable  =  value & 0b10111011;
      break; 

    case ARM_CORE0_MAILBOX_INT_CONTROL :
    case ARM_CORE1_MAILBOX_INT_CONTROL :
    case ARM_CORE2_MAILBOX_INT_CONTROL :
    case ARM_CORE3_MAILBOX_INT_CONTROL :
      
      if( ((addr&0xf)/4) >= vcpu->vm->vcpu_num ){
        log_debug("You cannot access registers of the cpu which is not assigned to the vm.\n");
        return -1;
      }

      src_vcpu = vcpu->vm->vcpu[(addr&0xf)/4];
      /* TODO */
      if(src_vcpu->state == VCPU_STATE_RUN){
        *(volatile uint32_t *)((src_vcpu->phys_cpu->cpu_id*4) 
            + addr&0xfffffff0) = value;
      }
      src_vcpu->vic.core_mbox_intr_enable  =  value;
      break;

    case ARM_CORE0_IRQ_PENDING :
    case ARM_CORE1_IRQ_PENDING :
    case ARM_CORE2_IRQ_PENDING :
    case ARM_CORE3_IRQ_PENDING :

    case ARM_CORE0_FIQ_PENDING :
    case ARM_CORE1_FIQ_PENDING :
    case ARM_CORE2_FIQ_PENDING :
    case ARM_CORE3_FIQ_PENDING :
      /*
       * These registers are read-only.
       * So we return -1.
       */
      log_debug("You cannot write to core interrupt pending registers.\n");
      return -1;
      break;

    default:
      log_debug("Illegal access to unavailable address\n");
      return -1;
  }
}

static void bcm2836_ic_reg_save(vcpu_t *vcpu){
  vcpu->vic.core_timer_intr_enable
      = (*(volatile uint32_t *)((vcpu->phys_cpu->cpu_id*4) + ARM_CORE0_CORE_TIMER_INT_CONTROL))
           & 0b01000100;

  vcpu->vic.core_mbox_intr_enable
      = (*(volatile uint32_t *)((vcpu->phys_cpu->cpu_id*4) + ARM_CORE0_MAILBOX_INT_CONTROL));
}

static void bcm2836_ic_reg_restore(vcpu_t *vcpu){
  *(volatile uint32_t *)((vcpu->phys_cpu->cpu_id*4) + ARM_CORE0_CORE_TIMER_INT_CONTROL) 
           |= vcpu->vic.core_timer_intr_enable & 0b10111011;

  *(volatile uint32_t *)((vcpu->phys_cpu->cpu_id*4) + ARM_CORE0_MAILBOX_INT_CONTROL) 
           |= vcpu->vic.core_mbox_intr_enable;
}

static void bcm2836_ic_reg_reset(void){

  *(volatile uint32_t *)ARM_CORE0_CORE_TIMER_INT_CONTROL &= 0b01000100;
  *(volatile uint32_t *)ARM_CORE1_CORE_TIMER_INT_CONTROL &= 0b01000100;
  *(volatile uint32_t *)ARM_CORE2_CORE_TIMER_INT_CONTROL &= 0b01000100;
  *(volatile uint32_t *)ARM_CORE3_CORE_TIMER_INT_CONTROL &= 0b01000100;

  /* TODO */
  *(volatile uint32_t *)ARM_CORE0_MAILBOX_INT_CONTROL = 0;
  *(volatile uint32_t *)ARM_CORE1_MAILBOX_INT_CONTROL = 0;
  *(volatile uint32_t *)ARM_CORE2_MAILBOX_INT_CONTROL = 0;
  *(volatile uint32_t *)ARM_CORE3_MAILBOX_INT_CONTROL = 0;
}

void virt_device_intr_init(void){
  int i;
  
  log_info("Init virtual device interrupt\n");

  for(i=1; i<GPU_INTERRUPT_NUM; i++){
    irq_vec_table[i] = NULL;
  }
}

int virt_device_intr_set(vm_t *vm){
  int i;
  log_info("vm->vic.assigned_gpu_irq : %#x",
      vm->vic.assigned_gpu_irq);

  for(i=1; i<GPU_INTERRUPT_NUM; i++){
    if(vm->vic.assigned_gpu_irq&(1<<i)  &&
        irq_vec_table[i] == NULL){

      irq_vec_table[i] = vm;
    }
    else
      return -1;
  }
}

void virt_device_intr_release(vm_t *vm){
  int i;
  for(i=1; i<GPU_INTERRUPT_NUM; i++){
    if(irq_vec_table[i] == vm)
      irq_vec_table[i] = NULL;
  }
}

void virt_intr_handler(vcpu_t *cur_vcpu){
  int i;
  int irq_vcpu_id = cur_vcpu->vm->vic.gpu_irq_route&0b11;
  int fiq_vcpu_id = (cur_vcpu->vm->vic.gpu_irq_route&0b1100)>>2;
  
  uint64_t cntv_tval_el0;
  READ_SYSREG(cntv_tval_el0, cntv_tval_el0);

  if(vcpu_core_irq_is_pending(cur_vcpu)){
    log_debug("core_irq cpu id : %d\n", irq_vcpu_id);
    log_debug("cntv_cval_el0 : %#8x, cntv_tval_el0 : %#8x\n",
        cur_vcpu->sysreg.cntv_cval_el0, cntv_tval_el0);
    vcpu_do_virq(cur_vcpu->vm->vcpu[irq_vcpu_id]);
    return;
  }
  log_info("GPU IRQ\n");
  /* Search gpu irq pending */
  for(i=0; i<64; i++){
    if(gpu_irq_is_pending(i)){
      if(irq_vec_table[i]!=NULL){
        if(irq_vec_table[i]->vic.fiq_index!=i){
          vcpu_do_virq(irq_vec_table[i]->vcpu[irq_vcpu_id]);
          return;
        }
        else{
          vcpu_do_vfiq(irq_vec_table[i]->vcpu[fiq_vcpu_id]);
          return;
        }
      }
    }else{
      hyp_panic("Not Found irq vector No.%d\n", i);
    }
  }
}


void virt_fiq_handler(vcpu_t *cur_vcpu){
  int i;
  
  if(vcpu_core_fiq_is_pending(cur_vcpu)){
    vcpu_do_vfiq(cur_vcpu);
    return;
  }
}
