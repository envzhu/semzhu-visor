#include "typedef.h"
#include "lib.h"
#include "asm_func.h"
#include "print.h"

static int num2str(char *dst, uint64_t num, char type, int prefix,int column);
static int vsprintf(char *buf, char *fmt, uint64_t *argv, int argc);

/* print value in decimal */
int putxval(unsigned long value, int column){
  char buf[9];

  num2str(buf, value, 'x', 1, column);
  puts(buf);

  return 0;
}

/* print value in decimal*/
int putdval(unsigned long value, int column){
  char buf[9];

  num2str(buf, value, 'd', 1, column);
  puts(buf);

  return 0;
}

/* dump memory in hexadecimal */
int dump(char *buf, long size)
{
  long i;

  if (size < 0) {
    puts("no data.\n");
    return -1;
  }
  for (i = 0; i < size; i++) {
    putxval(buf[i], 2);
    if ((i & 0xf) == 15) {
      puts("\n");
    } else {
      if ((i & 0xf) == 7) puts(" ");
      puts(" ");
    }
  }
  puts("\n");
  return 0;
}

/* convert number to string */
static int num2str(char *dst, uint64_t num, char type, int prefix,int column)
{
  char buf[30];
  char *p;

  p = buf + sizeof(buf) - 1;
  *(p--) = '\0';

  if (!num && !column)
    column++;

  switch(type){
    case 'd':
      while (num || column) {
        *(p--) = "0123456789"[num % 10];
        num /= 10;
        if (column) column--;
        if(p<buf)
          hyp_panic("The number which you want to print is too big to print.\n");
      }
      break;
    case 'x':
      while (num || column) {
        *(p--) = "0123456789abcdef"[num & 0xf];
        num >>= 4;
        if (column) column--;
        if(p<buf)
            hyp_panic("The number which you want to print is too big to print.\n");

      }
      if(prefix){
        *(p--) = 'x';
        *(p--) = '0';
      }
      break;
    case 'X':
      while (num || column) {
        *(p--) = "0123456789ABCDEF"[num & 0xf];
        num >>= 4;
        if (column) column--;
        if(p<buf)
            hyp_panic("The number which you want to print is too big to print.\n");
      }
      break;
    default:
      break;
  }

  strcpy(dst, p+1);

  return strlen(p+1);
}


/*
 * printf format:
 *  %|parameter|flags|width|.precision|length|type| 
 */
static int vsprintf(char *buf, char *fmt, uint64_t *argv, int argc){
  int len;
  int size;
  char parameter;
  /* flag */
  int prefix = 0; 
  int width;
  int precision;
  int length;
  char type;

  int argp = 0;

  size = 0;
  len = 0;

  while(*fmt){
    
    /* processing on format,'%' */
    if(*fmt=='%'){
      prefix = 0;
      width = 0;
      fmt++;
      
      /* get flag */
      switch(*fmt){
      case '-':
        fmt++;
        break;      
      case '+':
        fmt++;
        break;
      case ' ':
        fmt++;
        break;
      case '#':
        prefix = 1;
        fmt++;
        break;
      case '0':
        fmt++;
      default:
        goto end_prefix;
      }

  end_prefix:

      /* get width */
      while((*fmt >= '0') && (*fmt <= '9')){
          width *= 10;
          width = *(fmt++) - '0';
      }

      switch(*fmt){
      /* case number: */
      case 'd': /* decimal 0-9 */
      case 'x': /* hexadecimal 0-f */
      case 'X': /* hexadecimal 0-F */
        size = num2str(buf, argv[argp++], *fmt, prefix, width);
        break;
      /* case char: */
      case 'c': /* character */
        buf = (char)argv[argp++];
        size = 1;
        break;
      case 's': /* string */
        strcpy(buf, argv[argp]);
        buf += strlen(argv[argp++]);
        break;
      /* case escape sequence */
      case '%':
        *buf = '%';
        size = 1;
        break;
      default:
        break;
      }
      len += size;
      buf += size;
      fmt++;
    }else{
      *(buf++) = *(fmt++);
      len++;
    }
  }

  *buf = '\0'; /* Add '\0' at the end of string */

  return len;
}

int sprintf(char *buf, char *fmt, ...){
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);

  vsprintf(buf, fmt, &argv[2], 6);
}

int vprintf( char *fmt, uint64_t *argv, int argc){
  char buf[256];

  vsprintf(buf, fmt, argv, argc);
  
  puts(buf); 
}

int printf(char *fmt, ...){
  char buf[256];; 
  uint64_t argv[8];

  /* get 8 arguments */
  get_args(argv);
  
  vsprintf(buf, fmt, &argv[1], 7);
  
  puts(buf); 
}
