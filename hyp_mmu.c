/* 
 * hyp_mmu.c
 * Control Stage 2 MMU
 */

#include "typedef.h"
#include "lib.h"
#include "coproc_def.h"
#include "asm_func.h"
#include "hyp_mmu.h"

/* 
 * Stage 2 MMU control registers are :
 * - SCTLR_EL2
 * - VTTBR_EL2
 * - VTCR_EL2
 */

#define VTCR_RES  (1 << 31) // Reserved value
#define TTBL_VALID_MASK			  1
#define TTBL_TABLE_MASK		    0b10
#define MAX_PAGE_TABLE_NUM    512

typedef uint64_t ttbl_t;

static void map_ttbl(ttbl_t *l1_ttbl, phys_addr_t IA, phys_addr_t OA);
static ttbl_t *alloc_ttbl(void);

/* TODO : Support smp */
void mmu_init(void)
{
  uint32_t i;
  volatile uint32_t r;

  log_debug("Init Stage2 MMU\n");

  /* sctlr */
  READ_SYSREG(r, hcr_el2);
  r &=  ~HCR_VM; // Disable Stage 2 MMU
  WRITE_SYSREG(hcr_el2, r);
  asm volatile("isb");
  
  /* VTCR_EL2 */
  r = (0b01 << 31)  | // reserved bits
      (0b000 << 16) | // PA=32bits(4GB) ... options are 000=32bits(4GB), 001=36bits(64GB), 010=40bits(1TB),  ... Physical Address Size
      (0b00 << 14)  | // TG0=4k  ... options are 00=4KB,, 10=64KB,  ... Granule size for the corresponding VTTBR_EL2
      (0b00 << 12)  | // SH0=0 Non-shareable  .. options 00 = Non-shareable, 01 = Reserved, 10 = Outer Shareable, 11 = Inner Shareable
      (0b01 << 10)  | // ORGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write throw cacheable, 11 = Write Back Non-cacheable
      (0b01 << 8)   | // IRGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write throw cacheable, 11 = Write Back Non-cacheable
      (0b1  << 6)   | // SL0=1  ... Starting level of the VTCR_EL2 addressed region
      (25   << 0);    // T0SZ=25 (512G)  ... The region size is 2 POWER (64-T0SZ) bytes
  WRITE_SYSREG(vtcr_el2, r);
  asm volatile("dmb sy");
  asm volatile ("isb");
  
  /* sctlr */
  READ_SYSREG(r, hcr_el2);
  r |=  HCR_VM; // Enable Stage 2 MMU
  WRITE_SYSREG(hcr_el2, r);
  asm volatile("isb");
}

void mmu_switch(ttbl_t stage1_ttbr, ttbl_t stage2_ttbr){
  asm volatile("tlbi vmalle1");
}

void set_vttbr(uint64_t vttbr){
  /* clear pipeline */
  asm volatile("dsb sy");
  
  asm volatile("tlbi vmalle1");

  /* VTTBR_EL2 */
  asm volatile ("msr vttbr_el2, %0" : : "r" ((phys_addr_t)vttbr));
  asm volatile ("isb");
}

/**
 * @fn
 * @param (l1_ttbl) pointer to leval 1 page table address
 * @param (IA)  Intermediate physical address
 * @param (OA)  Physical address
 */
static void map_ttbl(ttbl_t *l1_ttbl, phys_addr_t IA, phys_addr_t OA){
  ttbl_t * l2_ttbl;
  ttbl_t * l3_ttbl;

  /* Setup level1 table */
  if(l1_ttbl[(IA>>30)&0x1ff]&TTBL_VALID_MASK){
    l2_ttbl = l1_ttbl[IA>>30]&0xFFFFFFFFF000;
  }else{
    l2_ttbl = alloc_ttbl();
    l1_ttbl[(IA>>30)&0x1ff] = (phys_addr_t)l2_ttbl | TTBL_TABLE_MASK | TTBL_VALID_MASK;
  }

  /* Setup level2 table */
  if(l2_ttbl[(IA>>21)&0x1ff]&TTBL_VALID_MASK){
    l3_ttbl = l2_ttbl[IA>>21]&0xFFFFFFFFF000;
  }else{
    l3_ttbl = alloc_ttbl();
    l2_ttbl[(IA>>21)&0x1ff] = (phys_addr_t)l3_ttbl | TTBL_TABLE_MASK | TTBL_VALID_MASK;
    //log_debug("%#x,\n",&l2_ttbl[IA>>21]);
  }
  
  /* Setup level3 block */
  l3_ttbl[(IA>>12)&0x1ff] = (0b100<<2) | OA  | TTBL_TABLE_MASK | TTBL_VALID_MASK |
                                  (1<<10) |   // AF, The Access flag
                                  (0b11<<6);  // S2AP = 11 RW ... options are 00=Nnne, 01=RO, 10=WO, 11=RW, ... Data access permissions
  
  //log_debug("l1_page_table addr : %#x\nl2_page_table addr : %#x, index:%#x \nl3_page_table addr : %#x\n", l1_ttbl, l2_ttbl, (IA>>21)&0x1ff, l3_ttbl);
  //log_debug("IPA : %#8x to PA : %#8x\n", IA, OA);
}

uint64_t map_page_table(uint64_t ttbr,  phys_addr_t IA_start,
                      phys_addr_t OA_start, uint64_t length){
  
  ttbl_t *l1_ttbl = (ttbl_t *)ttbr;
  phys_addr_t IA_t;
  phys_addr_t OA_t;

  IA_start  &= 0xFFFFFFFFF000;
  OA_start  &= 0xFFFFFFFFF000;
  if(length%0x1000 != 0){
    length &= 0xFFFFFFFFF000;
    length += 0x1000;
  }

  log_debug("map_page_table\nIA_start : %#8x\nOA_start : %#8x\nLength   : %#8x\n", IA_start, OA_start, length);
  
  if(!ttbr){
    hyp_panic("vttbr is zero!");
  }

  for(IA_t = IA_start, OA_t = OA_start;
        IA_t < (IA_start + length); IA_t += 0x1000, OA_t += 0x1000){
    
    map_ttbl(l1_ttbl, IA_t, OA_t);
  }

  return ttbr;
}

static ttbl_t *alloc_ttbl(void){
  static volatile __attribute__((aligned(4096))) ttbl_t page_tables[MAX_PAGE_TABLE_NUM][512];
  static int bottom_index = 0;
  
  if(bottom_index >= (MAX_PAGE_TABLE_NUM-1))
    hyp_panic("There is no free page_table\n");
  
  memset(&page_tables[bottom_index], 0, sizeof(page_tables[0]));
  return &page_tables[bottom_index++];
}

uint64_t alloc_vttbr(void){
  return (uint64_t)alloc_ttbl();
}

/**
 * @fn
 * @param (ttbr) pointer to leval 1 page table address
 */
void dump_ttbl(uint64_t ttbr){
  ttbl_t *l1_ttbl = (ttbl_t *)ttbr;
  ttbl_t *l2_ttbl;
  ttbl_t *l3_ttbl;

  log_debug("Dump page_table:\n");
  for(int i=0; i<512; i++){
    if(l1_ttbl[i]&TTBL_VALID_MASK){
      l2_ttbl = l1_ttbl[i]&0xFFFFFFFFF000;
      for(int j=0; j<512; j++){
        if(l2_ttbl[j]&TTBL_VALID_MASK){
          l3_ttbl = l2_ttbl[j]&0xFFFFFFFFF000;
    for(int k=0; k<512; k++){
      if(l3_ttbl[k]&TTBL_VALID_MASK){
        log_debug("IPA : %#8x to PA : %#8x\n", (i<<30)+(j<<21)+(k<<12), l3_ttbl[k]&0xFFFFFFFFF000);
      }
    }
        }
      }
    }
  }

  log_debug("Dump page_table End\n");
}

/* VA to PA Address Translation */

#define VA2PA_STAGE1		"s1"
#define VA2PA_STAGE12		"s12"
#define VA2PA_EL0		"e0"
#define VA2PA_EL1		"e1"
#define VA2PA_EL2		"e2"
#define VA2PA_EL3		"e3"
#define VA2PA_RD		"r"
#define VA2PA_WR		"w"
#define va2pa_at(stage, el, rw, va)	asm volatile(	\
					"at " stage el rw ", %0" \
					: : "r"(va) : "memory", "cc");


phys_addr_t el1va2ipa(phys_addr_t va){
  uint64_t par_el1;

  va2pa_at(VA2PA_STAGE1, VA2PA_EL1, VA2PA_RD, va);
  READ_SYSREG(par_el1, PAR_EL1);
  return par_el1&0xFFFFFFFFF000 | (va & 0x00000FFF);
}

phys_addr_t el1va2pa(phys_addr_t va){
  uint64_t par_el1;

  va2pa_at(VA2PA_STAGE12, VA2PA_EL1, VA2PA_RD, va);
  READ_SYSREG(par_el1, PAR_EL1);
  return par_el1&0xFFFFF000 | (va & 0x00000FFF);
}
