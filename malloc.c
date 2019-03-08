/* 
 * malloc.c
 * Memory Allcater
 * Buddy memory allocation
 */

#include "typedef.h"
#include "lib.h"
#include "asm_func.h"
#include "malloc.h"

extern uint8_t _freearea_start;
extern uint8_t _freearea_end;

#define MEM_BLOCK_NUM (uint64_t)(((phys_addr_t)&_freearea_end -(phys_addr_t) &_freearea_start) / MEM_BLOCK_SIZE)
#define MEM_BLOCK_SIZE  4096 // Byte per Block
#define MEMORY_POOL_SIZE (MEM_BLOCK_SIZE*MEM_BLOCK_NUM)

static volatile uint8_t *mem_pool = &_freearea_start;
static int free_top_index = 0;


void mem_init(void){
  free_top_index = 0;
  log_info("&_freearea_start : %#x, &_freearea_end : %#x\n", &_freearea_start, &_freearea_end);
  log_info("MEM_BLOCK_SIZE : %d, MEM_BLOCK_NUM : %d, MEMORY_POOL_SIZE : %d \n", MEM_BLOCK_SIZE, MEM_BLOCK_NUM, MEMORY_POOL_SIZE);
}

void *malloc(uint64_t size){
  int head_index;
  
  if(size %MEM_BLOCK_SIZE != 0){
    size = size - (size % MEM_BLOCK_SIZE) + MEM_BLOCK_SIZE;
  }
  
  if((free_top_index + size / MEM_BLOCK_SIZE) > (MEM_BLOCK_NUM-1))
    hyp_panic("There is no free page_table. size : %#x\n", size);
  
  head_index = free_top_index;
  free_top_index += size / MEM_BLOCK_SIZE;
  memset(&mem_pool[head_index*MEM_BLOCK_SIZE], 0, size);
  return &mem_pool[head_index*MEM_BLOCK_SIZE];
}
