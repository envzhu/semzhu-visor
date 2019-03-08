#ifndef _UART_H_INCLUDED_
#define _UART_H_INCLUDED_

void uart_init(void);                 /* init uart */
int uart_send_byte(unsigned char b);  /* send a data */
unsigned char uart_recv_byte(void);   /* recieve a data */

#endif
