/*
 * EMMC (External Mass Media Controller) driver
 */

#include "typedef.h"
#include "hardware_def.h"
#include "gpio.h"
#include "print.h"

#define put_hex(c) putxval(c, 0)
#define EMMC_BASE   (0x00300000)
#define EMMC_REG(x) *((volatile uint32_t *)PHY_PERI_ADDR(EMMC_BASE+x))

// EMMC Peripheral
#define EMMC_ARG2           EMMC_REG(0x00)
#define EMMC_BLKSIZECNT     EMMC_REG(0x04)
#define EMMC_ARG1           EMMC_REG(0x08)
#define EMMC_CMDTM          EMMC_REG(0x0C)
#define EMMC_RESP0          EMMC_REG(0x10)
#define EMMC_RESP1          EMMC_REG(0x14)
#define EMMC_RESP2          EMMC_REG(0x18)
#define EMMC_RESP3          EMMC_REG(0x1C)
#define EMMC_DATA           EMMC_REG(0x20)
#define EMMC_STATUS         EMMC_REG(0x24)
#define EMMC_CONTROL0       EMMC_REG(0x28)
#define EMMC_CONTROL1       EMMC_REG(0x2C)
#define EMMC_INTERRUPT      EMMC_REG(0x30)
#define EMMC_INT_MASK       EMMC_REG(0x34)
#define EMMC_INT_EN         EMMC_REG(0x38)
#define EMMC_CONTROL2       EMMC_REG(0x3C)

#define EMMC_FORCE_IRPT     EMMC_REG(0x50)
#define EMMC_BOOT_TIMEOUT   EMMC_REG(0x70)
#define EMMC_DBG_SEL        EMMC_REG(0x74)
#define EMMC_EXRDFIFO_CFG   EMMC_REG(0x80)
#define EMMC_EXRDFIFO_EN    EMMC_REG(0x84)
#define EMMC_TUNE_STEP      EMMC_REG(0x88)
#define EMMC_TUNE_STEPS_STD EMMC_REG(0x8c)
#define EMMC_TUNE_STEPS_DDR EMMC_REG(0x90)
#define EMMC_SPI_INT_SPT    EMMC_REG(0xf0)

#define EMMC_SLOTISR_VER    EMMC_REG(0xFC)

// command flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_CMD_INHIBIT      (1<<0)
#define SR_DAT_INHIBIT      (1<<1)
#define SR_DAT_ACTIVE       (1<<2)
#define SR_WRITE_AVAILABLE  (1<<8)
#define SR_READ_AVAILABLE   0x00000800

#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_CMD_DONE        (1<<0)
#define INT_DATA_DONE       (1<<1)
#define INT_BLOCK_GAP       (1<<2)
#define INT_WRITE_RDY       (1<<4)
#define INT_READ_RDY        (1<<5)
#define INT_CARD            (1<<8)
#define INT_RETUNE          (1<<12)
#define INT_BOOTACK         (1<<13)
#define INT_ENDBOOT         (1<<14)
#define INT_ERR             (1<<15)
#define INT_CMD_TIMEOUT     (1<<16)
#define INT_CCRC_ERR        (1<<17)
#define INT_CEND_ERR        (1<<18)
#define INT_CBAD_ERR        (1<<19)
#define INT_DATA_TIMEOUT    (1<<20)
#define INT_DCRC_ERR        (1<<21)
#define INT_DEND_ERR        (1<<22)
#define INT_ACMD_ERR        (1<<24)

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_HCTL_DWITDH      (1<<1)
#define C0_HCTL_HS_EN       (1<<2)
#define C0_HCTL_8BIT        (1<<5)
#define C0_GAP_STOP         (1<<16)
#define C0_GAP_RESTART      (1<<17)
#define C0_READWAIT_EN      (1<<18)
#define C0_GAP_IEN          (1<<19)
#define C0_SPI_MODE_EN      (1<<20)
#define C0_BOOT_EN          (1<<21)
#define C0_ALT_BOOT_EN      (1<<22)

#define C1_CLK_INTLEN       (1<<0)
#define C1_CLK_STABLE       (1<<1)
#define C1_CLK_EN           (1<<2)
#define C1_CLK_GENSEL       (1<<5)
// 6...7 : CLK_FREQ_MS2
// 8...15: CLK_FREQ8
//16...19: DATA_TOUNIT
#define C1_TOUNIT_MAX       0x000e0000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_SRST_HC          (1<<24)
#define C1_SRST_CMD         (1<<25)
#define C1_SRST_DATA        (1<<26)

// TUNE_STEP register settings
#define TS_DELAY_200        0b000
#define TS_DELAY_400        0b001
#define TS_DELAY_600        0b011
#define TS_DELAY_700        0b100
#define TS_DELAY_900        0b101
#define TS_DELAY_1100       0b111

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
// added by my driver
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

#define SD_OK                0
#define SD_TIMEOUT          -1
#define SD_ERROR -2

unsigned long sd_scr[2], sd_ocr, sd_rca, sd_err, sd_hv;

/**
 * Wait N microsec (ARM CPU only)
 */
void wait_cycles(unsigned int n)
{
    if(n) while(n--) { asm volatile("nop"); }
}

void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}

/**
 * Wait for data or command ready
 */
int sd_status(uint32_t mask)
{
    int cnt = 500000; 
    while((EMMC_STATUS & mask) && !(EMMC_INTERRUPT & INT_ERROR_MASK) && cnt--)
      wait_msec(1);
    return (cnt <= 0 || (EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(uint32_t mask)
{
    unsigned int r, m=mask | INT_ERROR_MASK;
    int cnt = 1000000; while(!(EMMC_INTERRUPT & m) && cnt--) wait_msec(1);
    r=EMMC_INTERRUPT;
    if(cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT) ) { EMMC_INTERRUPT=r; return SD_TIMEOUT; } else
    if(r & INT_ERROR_MASK) { EMMC_INTERRUPT=r; return SD_ERROR; }
    EMMC_INTERRUPT=mask;
    return 0;
}

/**
 * Send a command
 */
int sd_cmd(unsigned int code, unsigned int arg){
  int r = 0;
  sd_err=SD_OK;

  if(code&CMD_NEED_APP) {
    r = sd_cmd(CMD_APP_CMD|(sd_rca?CMD_RSPNS_48:0),sd_rca);
    if(sd_rca && !r) {
      log_debug("ERROR: failed to send SD APP command\n");
      sd_err=SD_ERROR;
      return 0;
    }
    code &= ~CMD_NEED_APP;
  }
  
  if(sd_status(SR_CMD_INHIBIT)){
    log_debug("ERROR: EMMC busy\n");
    sd_err= SD_TIMEOUT;
    return 0;
  }

  log_debug("EMMC: Sending command  code:%#8x, arg1:%#x\n", code, arg);
  EMMC_INTERRUPT=EMMC_INTERRUPT;EMMC_ARG1=arg; EMMC_CMDTM=code;
  if(code==CMD_SEND_OP_COND) wait_msec(1000); else
  if(code==CMD_SEND_IF_COND || code==CMD_APP_CMD) wait_msec(100);
  
  if((r=sd_int(INT_CMD_DONE))) {
    log_debug("ERROR: failed to send EMMC command\n");
    sd_err=r;
    return 0;
  }
  r=EMMC_RESP0;
  
  if(code==CMD_GO_IDLE || code==CMD_APP_CMD) return 0; else
  if(code==(CMD_APP_CMD|CMD_RSPNS_48)) return r&SR_APP_CMD; else
  if(code==CMD_SEND_OP_COND) return r; else
  if(code==CMD_SEND_IF_COND) return r==arg? SD_OK : SD_ERROR; else
  if(code==CMD_ALL_SEND_CID) {
    r|=EMMC_RESP3 | EMMC_RESP2 | EMMC_RESP1; return r; } else
  if(code==CMD_SEND_REL_ADDR) {
      sd_err=(((r&0x1fff))|((r&0x2000)<<6)|((r&0x4000)<<8)|((r&0x8000)<<8))&CMD_ERRORS_MASK;
      return r&CMD_RCA_MASK;
  }
  return r&CMD_ERRORS_MASK;
  // make gcc happy
  return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
int sd_readblock(uint64_t lba, void *buffer, uint64_t num)
{
  int r,c=0,d;
  uint32_t *buf=(uint32_t *)buffer;
  if(num<1) num=1;
  
  log_debug("sd_readblock lba : %#x, num : %#x\n", lba, num);
  if(sd_status(SR_DAT_INHIBIT)){
    sd_err=SD_TIMEOUT;
    return 0;
  }
  
  if(sd_scr[0] & SCR_SUPP_CCS) {
    if(num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
      sd_cmd(CMD_SET_BLOCKCNT,num);
      if(sd_err) return 0;
    }
    EMMC_BLKSIZECNT = (num << 16) | 512;
    sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI,lba);
    if(sd_err) return 0;
  } else {
    EMMC_BLKSIZECNT = (1 << 16) | 512;
  }
  
  for(c = 0; c < num; c++){
    if(!(sd_scr[0] & SCR_SUPP_CCS)) {
      sd_cmd(CMD_READ_SINGLE,(lba+c)*512);
      if(sd_err) return 0;
    }
    if((r=sd_int(INT_READ_RDY))){
      log_debug("ERROR: Timeout waiting for ready to read\n");
      sd_err=r;return 0;
    }
    for(d=0; d<128; d++)
      buf[d] = EMMC_DATA;
    buf+=128;
  }

  if((num > 1) 
      && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) 
      && (sd_scr[0] & SCR_SUPP_CCS))
    sd_cmd(CMD_STOP_TRANS,0);
  
  return sd_err!=SD_OK || c!=num? 0 : num*512;
}

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(uint64_t f)
{
  uint64_t d,c=41666666/f,x,s=32,h=0;
  int cnt = 100000;
  while((EMMC_STATUS & (SR_CMD_INHIBIT|SR_DAT_INHIBIT)) && cnt--) wait_msec(1);
  if(cnt<=0) {
    log_debug("ERROR: timeout waiting for inhibit flag\n");
    return SD_ERROR;
  }

  EMMC_CONTROL1 &= ~C1_CLK_EN; wait_msec(10);
  x=c-1; if(!x) s=0; else {
    if(!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
    if(!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
    if(!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
    if(!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
    if(!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
    if(s>0) s--;
    if(s>7) s=7;
  }
  if(sd_hv>HOST_SPEC_V2) d=c; 
  else d=(1<<s);
  if(d<=2) {d=2;s=0;}
  log_debug("sd_clk divisor : %#x, shift : %#x\n", d, s);
  if(sd_hv>HOST_SPEC_V2) h=(d&0x300)>>2;
  d=(((d&0x0ff)<<8)|h);
  EMMC_CONTROL1=(EMMC_CONTROL1&0xffff003f)|d; wait_msec(10);
  EMMC_CONTROL1 |= C1_CLK_EN; wait_msec(10);
  cnt=10000; while(!(EMMC_CONTROL1 & C1_CLK_STABLE) && cnt--) wait_msec(10);
  if(cnt<=0) {
    log_debug("ERROR: failed to get stable clock\n");
    return SD_ERROR;
  }
  return SD_OK;
}

/**
 * initialize EMMC to read SDHC card
 */
int sd_init(void){
  int64_t  r,cnt,ccs=0;

  // GPIO_CD
  GPFSEL4&=~(7<<(7*3));
  GPPUD = 2;wait_cycles(150);
  GPPUDCLK1=(1<<15); wait_cycles(150);
  GPPUD=0; GPPUDCLK1=0;
  GPHEN1 |= 1<<15;

  // GPIO_CLK, GPIO_CMD
  GPFSEL4 |= (7<<(8*3))|(7<<(9*3));
  GPPUD=2; wait_cycles(150); GPPUDCLK1=(1<<16)|(1<<17); wait_cycles(150); GPPUD=0; GPPUDCLK1=0;

  // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
  GPFSEL5 |= (7<<(0*3)) | (7<<(1*3)) | (7<<(2*3)) | (7<<(3*3));
  GPPUD = 2;
  wait_cycles(150);
  GPPUDCLK1 = (1<<18) | (1<<19) | (1<<20) | (1<<21);
  wait_cycles(150); GPPUD=0; GPPUDCLK1=0;

  sd_hv = (EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
  log_debug("EMMC: GPIO set up\n");
  // Reset the card.
  EMMC_CONTROL0 = 0; EMMC_CONTROL1 |= C1_SRST_HC;
  cnt=10000; do{wait_msec(10);} while( (EMMC_CONTROL1 & C1_SRST_HC) && cnt-- );
  if(cnt<=0) {
      log_debug("ERROR: failed to reset EMMC\n");
      return SD_ERROR;
  }
  log_debug("EMMC: reset OK\n");
  EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
  wait_msec(10);
  // Set clock to setup frequency.
  if((r=sd_clk(400000))) return r;
  EMMC_INT_EN   = 0xffffffff;
  EMMC_INT_MASK = 0xffffffff;
  sd_scr[0]=sd_scr[1]=sd_rca=sd_err=0;
  
  // CMD0
  sd_cmd(CMD_GO_IDLE,0);
  if(sd_err) return sd_err;

  // CMD8
  sd_cmd(CMD_SEND_IF_COND,0x000001AA);
  if(sd_err) return sd_err;
  
  // ACMD41
  cnt=6; r=0; 
  while(!(r&ACMD41_CMD_COMPLETE) && cnt--) {
    wait_cycles(400);
    r = sd_cmd(CMD_SEND_OP_COND,ACMD41_ARG_HC);
    log_debug("EMMC: CMD_SEND_OP_COND returned ");
    if(r&ACMD41_CMD_COMPLETE)
        log_debug("COMPLETE ");
    if(r&ACMD41_VOLTAGE)
        log_debug("VOLTAGE ");
    if(r&ACMD41_CMD_CCS)
        log_debug("CCS ");
    put_hex(r>>32);
    put_hex(r);
    log_debug("\n");
    if(sd_err!=SD_TIMEOUT && sd_err!=SD_OK ) {
        log_debug("ERROR: EMMC ACMD41 returned error\n");
        return sd_err;
    }
  }
  if(!(r&ACMD41_CMD_COMPLETE) || !cnt ) return SD_TIMEOUT;
  if(!(r&ACMD41_VOLTAGE)) return SD_ERROR;
  if(r&ACMD41_CMD_CCS) ccs=SCR_SUPP_CCS;

  sd_cmd(CMD_ALL_SEND_CID,0);

  sd_rca = sd_cmd(CMD_SEND_REL_ADDR,0);
  log_debug("EMMC: CMD_SEND_REL_ADDR returned %#x\n", sd_rca);
  
  if(sd_err) return sd_err;

  if((r=sd_clk(25000000))) return r;

  sd_cmd(CMD_CARD_SELECT,sd_rca);
  if(sd_err) return sd_err;

  if(sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
  EMMC_BLKSIZECNT = (1<<16) | 8;
  sd_cmd(CMD_SEND_SCR,0);
  if(sd_err) return sd_err;
  if(sd_int(INT_READ_RDY)) return SD_TIMEOUT;

  r=0; cnt=100000;
  while(r<2 && cnt) {
    if( EMMC_STATUS & SR_READ_AVAILABLE )
      sd_scr[r++] = EMMC_DATA;
    else
      wait_msec(1);
  }
  if(r!=2) return SD_TIMEOUT;
  if(sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
      sd_cmd(CMD_SET_BUS_WIDTH,sd_rca|2);
      if(sd_err) return sd_err;
      EMMC_CONTROL0 |= C0_HCTL_DWITDH;
  }
  // add software flag
  log_debug("EMMC: supports ");
  if(sd_scr[0] & SCR_SUPP_SET_BLKCNT)
      log_debug("SET_BLKCNT ");
  if(ccs)
      log_debug("CCS ");

  sd_scr[0]&=~SCR_SUPP_CCS;
  sd_scr[0]|=ccs;
  return SD_OK;
}


void sd_dump(void){
  uint8_t buffer[4096];
  if(sd_init()==SD_OK) {
    // read the master boot record after our bss segment
    if(sd_readblock(0,buffer,1)) {
        // dump it to serial console
      log_debug("SD : %s\n", buffer);
    }
  }
}
