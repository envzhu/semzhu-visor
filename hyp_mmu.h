#ifndef _HYP_MMU_H_INCLUDED_
#define _HYP_MMU_H_INCLUDED_

#include "typedef.h"

void mmu_init(void);
void set_vttbr(uint64_t vttbr);
uint64_t alloc_vttbr(void);
uint64_t map_page_table(uint64_t ttbr,  phys_addr_t IA_start,
                      phys_addr_t OA_start, uint64_t length);
void dump_ttbl(uint64_t ttbr);

phys_addr_t el1va2ipa(phys_addr_t va);
phys_addr_t el1va2pa(phys_addr_t va);

#endif
