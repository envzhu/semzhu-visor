#include "typedef.h"
#include "uart.h"
#include "spinlock.h"
#include "lib.h"

void *memset(void *b, int c, long len)
{
  char *p;
  for (p = b; len > 0; len--)
    *(p++) = c;
  return b;
}

void *memcpy(void *dst, const void *src, long len)
{
  char *d = dst;
  const char *s = src;
  for (; len > 0; len--)
    *(d++) = *(s++);
  return dst;
}

int memcmp(const void *b1, const void *b2, long len)
{
  const char *p1 = b1, *p2 = b2;
  for (; len > 0; len--) {
    if (*p1 != *p2)
      return (*p1 > *p2) ? 1 : -1;
    p1++;
    p2++;
  }
  return 0;
}

int strlen(const char *s)
{
  int len;
  for (len = 0; *s; s++, len++)
    ;
  return len;
}

char *strcpy(char *dst, const char *src)
{
  char *d = dst;
  for (;; dst++, src++) {
    *dst = *src;
    if (!*src) break;
  }
  return d;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 || *s2) {
    if (*s1 != *s2)
      return (*s1 > *s2) ? 1 : -1;
    s1++;
    s2++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, int len)
{
  while ((*s1 || *s2) && (len > 0)) {
    if (*s1 != *s2)
      return (*s1 > *s2) ? 1 : -1;
    s1++;
    s2++;
    len--;
  }
  return 0;
}


int putc(unsigned char c)
{
  if (c == '\n')
    uart_send_byte('\r');
  return uart_send_byte(c);
}

unsigned char getc(void)
{
  unsigned char c = uart_recv_byte();
  c = (c == '\r') ? '\n' : c;
  putc(c); /* echo back */
  return c;
}

static ser_locked = 0;
int puts(unsigned char *str)
{
  spin_lock(&ser_locked);

  while (*str)
    putc(*(str++));
  
  spin_unlock(&ser_locked);
  return 0;
}

int gets(unsigned char *buf)
{
  int i = 0;
  unsigned char c;
  do {
    c = getc();
    if (c == '\n')
      c = '\0';
    buf[i++] = c;
  } while (c);
  return i - 1;
}
