#include "typedef.h"
#include "uart.h"
#include "hardware_def.h"

#define AUX_BASE   (0x00215000)
#define AUX_REG(x) *((volatile uint32_t *)BCM2835_PERI_ADDR(AUX_BASE+x))
#define AUX_IRQ		AUX_REG(0x00)
#define AUX_ENABLE  AUX_REG(0x04)

// Mini UART Peripheral
#define MU_IO		  AUX_REG(0x40)
#define MU_IER		AUX_REG(0x44)
#define MU_IIR		AUX_REG(0x48)
#define MU_LCR		AUX_REG(0x4C)
#define MU_MCR		AUX_REG(0x50)
#define MU_LSR		AUX_REG(0x54)
#define MU_MSR		AUX_REG(0x58)
#define MU_SCRATCH		AUX_REG(0x5C)
#define MU_CNTL		AUX_REG(0x60)
#define MU_STAT		AUX_REG(0x64)
#define MU_BAUD		AUX_REG(0x68)

#define MU_LSR_TX_IDLE	(1 << 6)
#define MU_LSR_TX_EMPTY	(1 << 5)
#define MU_LSR_RX_RDY	  (1 << 0)

void uart_init(void)
{
    puts("\n");
}

int uart_is_send_enable(void)
{
  return (MU_LSR & MU_LSR_TX_IDLE) || (MU_LSR & MU_LSR_TX_EMPTY);
}

int uart_is_recv_enable(void)
{
  return MU_LSR & MU_LSR_RX_RDY;
}

int uart_send_byte(unsigned char c)
{
  while (!uart_is_send_enable());
  MU_IO = (uint32_t)c;
  return 0;
}

unsigned char uart_recv_byte(void)
{
  while(!uart_is_recv_enable());
  return (unsigned char)(MU_IO & 0xff);
}
