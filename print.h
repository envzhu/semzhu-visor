#ifndef _PRINT_H_INCLUDED_
#define _PRINT_H_INCLUDED_

#include "typedef.h"

int putxval(unsigned long value, int column); /* print value in hexadecimal */
int putdval(unsigned long value, int column); /* print value in decimal */
int dump(char *buf, long size);	/* dump memory in hexadecimal */

int sprintf(char *buf, char *fmt, ...);
int vprintf(char *fmt, uint64_t *argv, int argc);
int printf(char *fmt, ...);

#endif
