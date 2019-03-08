/* 
 * hyp_timer.c
 * Control EL2 physical timer
 */

#include "typedef.h"
#include "lib.h"
#include "log.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "smp_mbox.h"
#include "hyp_timer.h"

/*
 * Generic Timer registers 
 * - CNTKCTL_EL1    32-bit Counter-timer Kernel Control register
 * - CNTFRQ_EL0     32-bit Counter-timer Frequency register
 * - CNTPCT_EL0     64-bit Counter-timer Physical Count register
 * - CNTVCT_EL0     64-bit Counter-timer Virtual Count register
 * - CNTP_TVAL_EL0  32-bit Counter-timer Physical Timer TimerValue register
 * - CNTP_CTL_EL0   32-bit Counter-timer Physical Timer Controlregister
 * - CNTP_CVAL_EL0  64-bit Counter-timer Physical Timer CompareValue register
 * - CNTV_TVAL_EL0  32-bit Counter-timer Virtual Timer TimerValue register
 * - CNTV_CTL_EL0 1 32-bit Counter-timer Virtual Timer Control register
 * - CNTV_CVAL_EL0  64-bit Counter-timer Virtual Timer CompareValue register
 * - CNTVOFF_EL2    64-bit Counter-timer Virtual Offset register
 * - CNTHCTL_EL2    32-bit Counter-timer Hypervisor Control register
 * - CNTHP_TVAL_EL2 32-bit Counter-timer Hypervisor Physical Timer TimerValue register
 * - CNTHP_CTL_EL2  32-bit Counter-timer Hypervisor Physical Timer Control register
 * - CNTHP_CVAL_EL2 64-bit Counter-timer Hypervisor Physical Timer CompareValue register
 * - CNTPS_TVAL_EL1 32-bit Counter-timer Physical Secure Timer TimerValue register
 * - CNTPS_CTL_EL1  32-bit Counter-timer Physical Secure Timer Control register
 * - CNTPS_CVAL_EL1 64-bit Counter-timer Physical Secure Timer CompareValue register
 */

/*
 * Generic Timer registers for Hypervisor
 * - CNTFRQ_EL0     32-bit Counter-timer Frequency register
 * - CNTVCT_EL0     64-bit Counter-timer Virtual Count register
 * - CNTV_TVAL_EL0  32-bit Counter-timer Virtual Timer TimerValue register
 * - CNTV_CTL_EL0 1 32-bit Counter-timer Virtual Timer Control register
 * - CNTV_CVAL_EL0  64-bit Counter-timer Virtual Timer CompareValue register
 * - CNTVOFF_EL2    64-bit Counter-timer Virtual Offset register
 * - CNTHCTL_EL2    32-bit Counter-timer Hypervisor Control register
 * - CNTHP_TVAL_EL2 32-bit Counter-timer Hypervisor Physical Timer TimerValue register
 * - CNTHP_CTL_EL2  32-bit Counter-timer Hypervisor Physical Timer Control register
 * - CNTHP_CVAL_EL2 64-bit Counter-timer Hypervisor Physical Timer CompareValue register
 */

#define TIMER_EVENT_NUM 16
#define TIMER_CLOCKS_PER_TICKS 0x10
#define TIMER_CLOCKS_PER_MSEC (1000000000 /1000) 
#define TIMER_TICKS_PER_MSEC (TIMER_CLOCKS_PER_MSEC/TIMER_CLOCKS_PER_TICKS) 
#define DEFAULT_TIMER_MSEC  100
#define DEFAULT_TIMER_CLOCKS TIMER_CLOCKS_PER_MSEC*DEFAULT_TIMER_MSEC
#define DEFAULT_TIMER_TICKS TIMER_TICKS_PER_MSEC*DEFAULT_TIMER_MSEC

#define CNTx_CTL_ENABLE   1
#define CNTx_CTL_IMASK    (1<<1)
#define CNTx_CTL_ISTATUS  (1<<2)

typedef struct _timer_event_t{
  int used;
  int64_t msec;
  uint64_t arg;
  void (*func) (pcpu_t *phys_cpu, uint64_t arg);
}timer_event_t;
timer_event_t timer_event_table[CPU_NUM][TIMER_EVENT_NUM];

static int hyp_tiemr_is_running[CPU_NUM];
static int hyp_timer_spinlock = 0;


void hyp_timer_core_init(pcpu_t *phys_cpu){
  hyp_tiemr_is_running[phys_cpu->cpu_id] = 0;
  *(uint32_t *)0x40000040 &= ~(1<<2);/* TODO */
  WRITE_SYSREG(CNTHP_CTL_EL2, 0);
}

/* Start hyp timer on current physical cpu  */
static void _hyp_timer_core_start(pcpu_t *phys_cpu){
  log_info("Start hyp timer in the physical cpu; cpu id : %d\n",
      phys_cpu->cpu_id);

  hyp_tiemr_is_running[phys_cpu->cpu_id]=1;
  WRITE_SYSREG(CNTHP_CVAL_EL2, 0);
  WRITE_SYSREG(CNTHP_TVAL_EL2,   DEFAULT_TIMER_TICKS);
  *(uint32_t *)0x40000040 = (1<<2);
  WRITE_SYSREG(CNTHP_CTL_EL2, CNTx_CTL_ENABLE);
}

/* Stop hyp timer on current physical cpu  */
static void _hyp_timer_core_stop(pcpu_t *phys_cpu){
  log_info("Stop hyp timer in the physical cpu; cpu id : %d\n",
      phys_cpu->cpu_id);

  hyp_tiemr_is_running[phys_cpu->cpu_id] = 0;
  *(uint32_t *)0x40000040 &= ~(1<<2);
  WRITE_SYSREG(CNTHP_CTL_EL2, 0);
}

/* Start hyp timer */
static void hyp_timer_start(pcpu_t *phys_cpu){
  
  if(phys_cpu == get_current_phys_cpu()){
    _hyp_timer_core_start(phys_cpu);
  }else{
    /* 
     * If start hyp timer of another cpu, wake up the cpu
     * because a cpu cannot access to hyp timer registers of another cpu.
     */
    smp_send_mailbox(phys_cpu->cpu_id, MAIL_TYPE_TIMER_START);
  }
}

/* Stop hyp timer */
static void hyp_timer_stop(pcpu_t *phys_cpu){
  
  if(phys_cpu == get_current_phys_cpu()){
    _hyp_timer_core_stop(phys_cpu);
  }else{
    /* 
     * If stop hyp timer of another cpu, wake up the cpu
     * because a cpu cannot access to hyp timer registers of another cpu.
     */
    smp_send_mailbox(phys_cpu->cpu_id, MAIL_TYPE_TIMER_STOP);
  }
}

uint64_t hyp_timer_get_clocks_num(int64_t msec){
  return TIMER_CLOCKS_PER_MSEC*msec;
}

uint64_t hyp_timer_tick2msec(int64_t tick){
  return tick/TIMER_TICKS_PER_MSEC;
}

static void timer_event_intr(pcpu_t *phys_cpu);
void hyp_timer_intr(pcpu_t *phys_cpu){
  int i=0;
  uint64_t cval, tval;
  READ_SYSREG(cval, CNTHP_CVAL_EL2);
  READ_SYSREG(tval, CNTHP_TVAL_EL2);
  log_info("before cval : %d, tval : %#8x\n", cval, tval);
  for(i=0; i<1000;i++){}
  READ_SYSREG(cval, CNTHP_CVAL_EL2);
  READ_SYSREG(tval, CNTHP_TVAL_EL2);
  log_info("after  cval : %d, tval : %d\n", cval, tval);

  WRITE_SYSREG(CNTHP_TVAL_EL2, DEFAULT_TIMER_TICKS);
  READ_SYSREG(cval, CNTHP_CVAL_EL2);
  READ_SYSREG(tval, CNTHP_TVAL_EL2);
  log_info("after1  cval : %d, tval : %d\n", cval, tval);

  WRITE_SYSREG(CNTHP_CTL_EL2, CNTx_CTL_ENABLE);

  timer_event_intr(phys_cpu);
}

void timer_event_init(void){
  int i, j;

  hyp_timer_spinlock = 0;

  for(i=0; i<CPU_NUM; i++){
    for(j=0; j<TIMER_EVENT_NUM; j++){
      timer_event_table[i][j].used = 0;
      timer_event_table[i][j].msec = 0;
      timer_event_table[i][j].func = NULL;
    }
  }
}

void timer_event_add(pcpu_t *phys_cpu, 
      void (*func)(pcpu_t *phys_cpu, uint64_t arg), int64_t msec, uint64_t arg){
  int i;
  timer_event_t *tp;

  spin_lock(&hyp_timer_spinlock);

  if(hyp_tiemr_is_running[phys_cpu->cpu_id] ==0)
    hyp_timer_start(phys_cpu);

  /* Find a free timer_event_t block */
  for(i=0; i<TIMER_EVENT_NUM; i++){
    if(timer_event_table[phys_cpu->cpu_id][i].used == 0)
      break;
  }

  if(i == TIMER_EVENT_NUM)
    hyp_panic("Not found a free timer_event_t block\n");
  
  timer_event_table[phys_cpu->cpu_id][i].used = 1;
  timer_event_table[phys_cpu->cpu_id][i].msec = msec;
  timer_event_table[phys_cpu->cpu_id][i].arg  = arg;
  timer_event_table[phys_cpu->cpu_id][i].func = func;

  spin_unlock(&hyp_timer_spinlock);
}

void timer_event_remove(pcpu_t *phys_cpu, void (*func) (void)){
  int i;

  spin_lock(&hyp_timer_spinlock);

  if(func == NULL){
    hyp_panic("You cannot remove NULL timer event from  the queue");
    return;
  }

  /* Find the timer_event_t block that func is belong to */
  for(i = 0; i < TIMER_EVENT_NUM; i++){
      if(timer_event_table[phys_cpu->cpu_id][i].func == func)
        break;
  }

  if(i == TIMER_EVENT_NUM){
    hyp_panic("You cannot remove timer event whichi is not in the queue from  the queue");
    return;
  }

  timer_event_table[phys_cpu->cpu_id][i].used = 0;
  timer_event_table[phys_cpu->cpu_id][i].msec = 0;
  timer_event_table[phys_cpu->cpu_id][i].arg  = 0;
  timer_event_table[phys_cpu->cpu_id][i].func = NULL;
  
  spin_unlock(&hyp_timer_spinlock);
}

static void timer_event_intr(pcpu_t *phys_cpu){
  int i;
  #define tp timer_event_table[phys_cpu->cpu_id][i]
  
  spin_lock(&hyp_timer_spinlock);

  log_debug("timer_event_intr()\n");

  /* Find active timer_event_t blocks */
  for(i=0; i<TIMER_EVENT_NUM;i++){
    if(tp.used){
      tp.msec -= DEFAULT_TIMER_MSEC;
      log_info("timer event %d used  : %d, msec : %d\n",
          i, tp.used, tp.msec);
      if(tp.msec<=0){
        log_info("delete timer event[%d][%d]\n",
            phys_cpu->cpu_id, i);
        spin_unlock(&hyp_timer_spinlock);
        tp.func(phys_cpu, tp.arg);
        spin_lock(&hyp_timer_spinlock);

        tp.used = 0;
        tp.msec = 0;
        tp.arg  = 0;
        tp.func = NULL;
      }
    }
  }
  
  for(i=0; i<TIMER_EVENT_NUM;i++){
    if(tp.used)
      break;
  }

  /* If there is no timer event, stop hyp_timer. */
  if(i==TIMER_EVENT_NUM)
    hyp_timer_stop(phys_cpu);

  spin_unlock(&hyp_timer_spinlock);
}
