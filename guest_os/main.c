#include "lib.h"

#define INTR_ENABLE   asm volatile("msr  daifclr, #0b1111")
#define INTR_DISABLE  asm volatile("msr  daifset, #0b1111")

char *hyp_msg = (char *)(0x100000);
char str[] = "hello\n"; 

void main(void){
  asm volatile("mrs  x3, currentEL");
  //INTR_ENABLE;

  memcpy(hyp_msg, str, sizeof(str));
  asm volatile("hvc #0");
  
  INTR_DISABLE;

  while(1){
  }
  
  asm volatile("wfi");
  asm volatile("hvc #1");
}
