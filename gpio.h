#ifndef _GPIO_H
#define _GPIO_H

#include "typedef.h"
#include "hardware_def.h"

// GPIO Peripheral
#define GPIO_BASE	(0x00200000)
#define GPIO_REG(x) *((volatile uint32_t *)PHY_PERI_ADDR(GPIO_BASE+x))

#define GPFSEL0		GPIO_REG(0x00)
#define GPFSEL1		GPIO_REG(0x04)
#define GPFSEL2		GPIO_REG(0x08)
#define GPFSEL3		GPIO_REG(0x0c)
#define GPFSEL4		GPIO_REG(0x10)
#define GPFSEL5		GPIO_REG(0x14)
#define GPSET0		GPIO_REG(0x1c)
#define GPSET1		GPIO_REG(0x20)
#define GPCLR0		GPIO_REG(0x28)
#define GPCLR1		GPIO_REG(0x2c)
#define GPLEV0		GPIO_REG(0x34)
#define GPLEV1		GPIO_REG(0x38)
// for GPFSEL mask
// use AND mask
#define GPFSEL_MASK_IN(n)	(~(volatile uint32)(0x07 << ((n % 10) * 3)))
// use OR mask
#define GPFSEL_MASK_OUT(n)	(0x01 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT0(n)	(0x04 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT1(n)	(0x05 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT2(n)	(0x06 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT3(n)	(0x07 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT4(n)	(0x03 << ((n % 10) * 3))
#define GPFSEL_MASK_ALT5(n)	(0x02 << ((n % 10) * 3))
// GPIO PULLUP/DOWN
#define GPPUD 		GPIO_REG(0x94)
#define GPPUDCLK0	GPIO_REG(0x98)
#define GPPUDCLK1	GPIO_REG(0x9C)


#define GPHEN0 GPIO_REG(0x64)
#define GPHEN1 GPIO_REG(0x68)

#endif
