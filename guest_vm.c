#include "vm.h"
#include "virt_mmio.h"
#include "schedule.h"
#include "virq.h"


extern uint8_t _linux_img_start;
extern uint8_t _linux_img_end;
extern uint8_t _bcm2837_rpi_3_b_img_start;
extern uint8_t _bcm2837_rpi_3_b_img_end;

extern uint8_t _initrd_img_start;
extern uint8_t _initrd_img_end;

extern uint8_t _kozos_img_start;
extern uint8_t _kozos_img_end;

extern uint8_t _sampleos_img_start;
extern uint8_t _sampleos_img_end;

mmp_t linux_mmp[] = {
    {.mem_start = 0, .mem_end = 0x7FFFF, .flag = 0},
    {.mem_start = 0x80000, .mem_end = 0x800000 - 1, .img_start = &_linux_img_start, .img_end = &_linux_img_end, .flag = MEM_VM_IMG},
    {.mem_start = 0x800000, .mem_end = 0x800000 + 0x200000 - 1, .img_start = &_bcm2837_rpi_3_b_img_start, .img_end = &_bcm2837_rpi_3_b_img_end, .flag = MEM_VM_IMG},
    {.mem_start = 0x800000 + 0x200000, .mem_end = 0x8000000 - 1, .flag = 0},
    {.mem_start = 0x8000000, .mem_end = 0x10000000 - 1, .img_start = &_initrd_img_start, .img_end = &_initrd_img_end, .flag = MEM_VM_IMG},
    //{.mem_start = 0x6000000, .mem_end = 0xFFFFFFF, .img_start = &_initrd_img_start, .img_end = &_initrd_img_end, .flag = MEM_VM_IMG}},
    // {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
};

mmp_t kozos_mmp[] = {
    {.mem_start = 0, .mem_end = 0xFFFFF, .img_start = &_kozos_img_start, .img_end = &_kozos_img_end, .flag = MEM_VM_IMG},
    {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
};

mmp_t sample_mmp[] = {
    {.mem_start = 0, .mem_end = 0x7FFFF, .flag = 0},
    {.mem_start = 0x80000, .mem_end = 0xFFFFF, .img_start = &_sampleos_img_start, .img_end = &_sampleos_img_end, .flag = MEM_VM_IMG},
    {.mem_start = 0x100000, .mem_end = 0x100FFF, .flag = MEM_HYP_VM_MSG},
};

void init_vm_create(void){
  /* Create vm */
  vm_create("linux1", 1, &fcfs_scheduler, 6, 0x80000,linux_mmp, sizeof(linux_mmp)/sizeof(linux_mmp[0]), 0, VIRT_INTR_UART, VIRT_MMIO_PL011|VIRT_MMIO_AUX, 0x000fffff00000000);
  // vm_create("kozos1", 1, &fcfs_scheduler, 2, 0x0000, kozos_mmp, sizeof(kozos_mmp) / sizeof(kozos_mmp[0]), 0, 0, 0, 0);
  // vm_create("sample1", 1, &fcfs_scheduler, 3, 0x80000, sample_mmp, sizeof(sample_mmp)/sizeof(sample_mmp[0]), 0, 0, 0, 0);
}
