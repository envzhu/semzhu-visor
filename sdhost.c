/*
 * BCM2835 SDHOST driver
 */

#include "typedef.h"
#include "hardware_def.h"
#include "gpio.h"
#include "print.h"


#define SDHOST_BASE   (0x00202000)
#define SDHOST_REG(x) *((volatile uint32_t *)PHY_PERI_ADDR(SDHOST_BASE+x))

#define msleep(a) udelay(a * 1000)

#define SDHOST_CMD    SDHOST_REG(0x00) /* Command to SD card              - 16 R/W */
#define SDHOST_ARG    SDHOST_REG(0x04) /* Argument to SD card             - 32 R/W */
#define SDHOST_TOUT   SDHOST_REG(0x08) /* Start value for timeout counter - 32 R/W */
#define SDHOST_CDIV   SDHOST_REG(0x0c) /* Start value for clock divider   - 11 R/W */
#define SDHOST_RSP0   SDHOST_REG(0x10) /* SD card response (31:0)         - 32 R   */
#define SDHOST_RSP1   SDHOST_REG(0x14) /* SD card response (63:32)        - 32 R   */
#define SDHOST_RSP2   SDHOST_REG(0x18) /* SD card response (95:64)        - 32 R   */
#define SDHOST_RSP3   SDHOST_REG(0x1c) /* SD card response (127:96)       - 32 R   */
#define SDHOST_HSTS   SDHOST_REG(0x20) /* SD host status                  - 11 R/W */
#define SDHOST_VDD    SDHOST_REG(0x30) /* SD card power control           -  1 R/W */
#define SDHOST_EDM    SDHOST_REG(0x34) /* Emergency Debug Mode            - 13 R/W */
#define SDHOST_HCFG   SDHOST_REG(0x38) /* Host configuration              -  2 R/W */
#define SDHOST_HBCT   SDHOST_REG(0x3c) /* Host byte count (debug)         - 32 R/W */
#define SDHOST_DATA   SDHOST_REG(0x40) /* Data to/from SD card            - 32 R/W */
#define SDHOST_HBLC   SDHOST_REG(0x50) /* Host block count (SDIO/SDHC)    -  9 R/W */

#define SDHOST_CMD_CMD_MASK       0x3f
#define SDHOST_CMD_READ_CMD			  (1<<6)
#define SDHOST_CMD_WRITE_CMD			(1<<7)
#define SDHOST_CMD_LONG_RESPONSE  (1<<9)
#define SDHOST_CMD_NO_RESPONSE    (1<<10)
#define SDHOST_CMD_BUSYWAIT       (1<<11)
#define SDHOST_CMD_FAIL_FLAG			(1<<14)
#define SDHOST_CMD_NEW_FLAG			  (1<<15)

#define SDCDIV_MAX_CDIV			0x7ff

#define SDHOST_HSTS_DATA_FLAG     (1<<0)
#define SDHOST_HSTS_FIFO_ERROR    (1<<3)
#define SDHOST_HSTS_CRC7_ERROR    (1<<4)
#define SDHOST_HSTS_CRC16_ERROR   (1<<5)
#define SDHOST_HSTS_CMD_TIME_OUT  (1<<6)
#define SDHOST_HSTS_REW_TIME_OUT  (1<<7)
#define SDHOST_HSTS_SDIO_IRPT     (1<<8)
#define SDHOST_HSTS_BLOCK_IRPT    (1<<9)
#define SDHOST_HSTS_BUSY_IRPT     (1<<10)


#define SDHOST_HSTS_CLEAR_MASK		(SDHOST_HSTS_BUSY_IRPT | \
					 SDHOST_HSTS_BLOCK_IRPT | \
					 SDHOST_HSTS_SDIO_IRPT | \
					 SDHOST_HSTS_REW_TIME_OUT | \
					 SDHOST_HSTS_CMD_TIME_OUT | \
					 SDHOST_HSTS_CRC16_ERROR | \
					 SDHOST_HSTS_CRC7_ERROR | \
					 SDHOST_HSTS_FIFO_ERROR)

#define SDHOST_HSTS_TRANSFER_ERROR_MASK	(SDHOST_HSTS_CRC7_ERROR | \
					 SDHOST_HSTS_CRC16_ERROR | \
					 SDHOST_HSTS_REW_TIME_OUT | \
					 SDHOST_HSTS_FIFO_ERROR)

#define SDHOST_HSTS_ERROR_MASK		(SDHOST_HSTS_CMD_TIME_OUT | \
					 SDHOST_HSTS_TRANSFER_ERROR_MASK)

#define SDHOST_HCFG_BUSY_IRPT_EN	BIT(10)
#define SDHOST_HCFG_BLOCK_IRPT_EN	BIT(8)
#define SDHOST_HCFG_SDIO_IRPT_EN	BIT(5)
#define SDHOST_HCFG_DATA_IRPT_EN	BIT(4)
#define SDHOST_HCFG_SLOW_CARD	BIT(3)
#define SDHOST_HCFG_WIDE_EXT_BUS	BIT(2)
#define SDHOST_HCFG_WIDE_INT_BUS	BIT(1)
#define SDHOST_HCFG_REL_CMD_LINE	BIT(0)

#define SDHOST_VDD_POWER_OFF		0
#define SDHOST_VDD_POWER_ON		1

#define SDHOST_EDM_FORCE_DATA_MODE	BIT(19)
#define SDHOST_EDM_CLOCK_PULSE	BIT(20)
#define SDHOST_EDM_BYPASS		BIT(21)

#define SDHOST_EDM_FIFO_FILL_SHIFT	4
#define SDHOST_EDM_FIFO_FILL_MASK	0x1f


#define SDHOST_EDM_WRITE_THRESHOLD_SHIFT	9
#define SDHOST_EDM_READ_THRESHOLD_SHIFT	  14
#define SDHOST_EDM_THRESHOLD_MASK		      0x1f

#define SDHOST_EDM_FSM_IDENTMODE    0x0
#define SDHOST_EDM_FSM_DATAMODE     0x1
#define SDHOST_EDM_FSM_READDATA     0x2
#define SDHOST_EDM_FSM_WRITEDATA    0x3
#define SDHOST_EDM_FSM_READWAIT     0x4
#define SDHOST_EDM_FSM_READCRC      0x5
#define SDHOST_EDM_FSM_WRITECRC     0x6
#define SDHOST_EDM_FSM_WRITEWAIT1   0x7
#define SDHOST_EDM_FSM_POWERDOWN    0x8
#define SDHOST_EDM_FSM_POWERUP      0x9
#define SDHOST_EDM_FSM_WRITESTART1  0xa
#define SDHOST_EDM_FSM_WRITESTART2  0xb
#define SDHOST_EDM_FSM_GENPULSES    0xc
#define SDHOST_EDM_FSM_WRITEWAIT2   0xd
#define SDHOST_EDM_FSM_STARTPOWDOWN 0xf
#define SDHOST_EDM_FSM_MASK		      0xf

#define SDDATA_FIFO_WORDS	16

#define FIFO_READ_THRESHOLD	4
#define FIFO_WRITE_THRESHOLD	4
#define SDDATA_FIFO_PIO_BURST	8

#define SDHST_TIMEOUT_MAX_USEC	100000

#define SDCMD0 0 
int sd_busy;

static void bcm2835_dumpregs(struct bcm2835_host *host)
{
	lof("=========== SDHOST REGISTER DUMP ==========\n");
	lof("SDHOST_CMD  %#8x\n", SDHOST_CMD);
	lof("SDHOST_ARG  %#8x\n", SDHOST_ARG);
	lof("SDHOST_TOUT %#8x\n", SDHOST_TOUT);
	lof("SDHOST_CDIV %#8x\n", SDHOST_CDIV);
	lof("SDHOST_RSP0 %#8x\n", SDHOST_RSP0);
	lof("SDHOST_RSP1 %#8x\n", SDHOST_RSP1);
	lof("SDHOST_RSP2 %#8x\n", SDHOST_RSP2);
	lof("SDHOST_RSP3 %#8x\n", SDHOST_RSP3);
	lof("SDHOST_HSTS %#8x\n", SDHOST_HSTS);
	lof("SDHOST_VDD  %#8x\n", SDHOST_VDD);
	lof("SDHOST_EDM  %#8x\n", SDHOST_EDM);
	lof("SDHOST_HCFG %#8x\n", SDHOST_HCFG);
	lof("SDHOST_HBCT %#8x\n", SDHOST_HBCT);
	lof("SDHOST_HBLC %#8x\n", SDHOST_HBLC);
	lof("===========================================\n");
}

static void bcm2835_reset_internal(struct bcm2835_host *host)
{


  /* Set GPIO to SDHOST */
  // GPIO_CLK, GPIO_CMD
   GPFSEL4 |= (4<<(8*3))|(4<<(9*3));

  // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
  GPFSEL5 |=(4<<(0*3)) | (4<<(1*3)) | (4<<(2*3)) | (4<<(3*3));

	SDHOST_VDD = SDHOST_VDD_POWER_OFF;
  SDHOST_CMD = 0;
  SDHOST_ARG = 0;
  /* Set timeout to a big enough value so we don't hit it */
  SDHOST_TOUT = 0xF00000;
  SDHOST_CDIV = 0;
  /* Clear status register */
  SDHOST_HSTS = 0x7F9;
  SDHOST_HCFG = 0;
  SDHOST_HBCT = 0;
  SDHOST_HBLC = 0;

	msleep(20);
	SDHOST_VDD = SDHOST_VDD_POWER_ON;
	/* Wait for all components to go through power on cycle */
	msleep(20);
	
	//SDHOST_HCFG = host->hcfg;
  //SDHOST_CDIV = host->cdiv;
}

/**
 * Send a command
 */
int sd_sendcmd(uint16_t cmd, uint32_t arg)
{
  SDHOST_ARG = arg;
  SDHOST_CMD = cmd& SDHOST_CMD_CMD_MASK | SDHOST_CMD_NEW_FLAG;
  return 0;
}

/**
 * get a response from sd card
 */
int sd_get_res(unsigned int code, unsigned int arg)
{
  r =  SDHOST_RSP0;
  return 0;
}

/**
 * transfer data to sd card
 */
int sd_trans_data(uint64_t block_size, uint64_t block_num)
{
  SDHBCT = block_size;
  SDHBLC = block_num;
  return 0;
}

/**
 * recieve data from sd card
 */
int sd_recv_data(uint64_t block_size, uint64_t block_num, uint32_t *dst)
{
  uint64_t i;
  
  sd_cmd(17|SDHOST_CMD_READ_CMD, 0);
  DHBCT = block_size;
  SDHBLC = block_num;
  
  for(i=0; i<block_size/sizeof(uint32_t); i++) {
    buf[i] = SDHOST_DATA;
  }
  return 0;
}

int sd_cmd(uint16_t cmd, uint32_t arg)
{
  int r=0;
  if(code&CMD_NEED_APP) {
    r=sd_cmd(CMD_APP_CMD|(sd_rca?CMD_RSPNS_48:0),sd_rca);
    code &= ~CMD_NEED_APP;
  }
  // check Error
  log_debug("EMMC: Sending command  code:%#8x, arg1:%#x\n", code, arg);
  EMMC_INTERRUPT=EMMC_INTERRUPT;EMMC_ARG1=arg; EMMC_CMDTM=code;
  if(code==CMD_SEND_OP_COND) wait_msec(1000); else
  if(code==CMD_SEND_IF_COND || code==CMD_APP_CMD) wait_msec(100);
  if((r=sd_int(INT_CMD_DONE))) {log_debug("ERROR: failed to send EMMC command\n");sd_err=r;return 0;}
  r=EMMC_RESP0;
  
  if(code==CMD_GO_IDLE || code==CMD_APP_CMD) return 0; else
  if(code==(CMD_APP_CMD|CMD_RSPNS_48)) return r&SR_APP_CMD; else
  if(code==CMD_SEND_OP_COND) return r; else
  if(code==CMD_SEND_IF_COND) return r==arg? SD_OK : SD_ERROR; else
  if(code==CMD_ALL_SEND_CID) {r|=EMMC_RESP3; r|=EMMC_RESP2; r|=EMMC_RESP1; return r; } else
  if(code==CMD_SEND_REL_ADDR) {
      sd_err=(((r&0x1fff))|((r&0x2000)<<6)|((r&0x4000)<<8)|((r&0x8000)<<8))&CMD_ERRORS_MASK;
      return r&CMD_RCA_MASK;
  }
  return r&CMD_ERRORS_MASK;
  // make gcc happy
  return 0;
}