OUTPUT_ARCH(elf64-littleaarch64)
ENTRY(startup_entry)

PROVIDE(_stack_size = 0x8000);

SECTIONS
{
  . = 0x80000;
  
  .text : { *(.text.boot) *(.text*) }

  .vector : { KEEP(*(.vector)) }

  . = ALIGN(0x1000);
  _rodata_start = .;
  .rodata : { *(.rodata*) }
  _rodata_end = .;
  
  . = ALIGN(0x1000);
  _sampleos_img_start = .;
  .gusetos : { sampleOS-img.o(.data) }
  _sampleos_img_end = .;

  . = ALIGN(0x1000);
  _kozos_img_start = .;
  .gusetos : { kozos-img.o(.data) }
  _kozos_img_end = .; 

  . = ALIGN(0x1000);
  _linux_img_start = .;
  .linux-img : { linux-img.o(.data) }
  _linux_img_end = .;
  
  . = ALIGN(0x1000);
  _bcm2837_rpi_3_b_img_start = .;
  .bcm2837_rpi_3_b_img : { bcm2837-rpi-3-b-img.o(.data) }
  _bcm2837_rpi_3_b_img_end = .;

   . = ALIGN(0x1000);
  _initrd_img_start = .;
  .initrd_img : { initrd-img.o(.data) }
  _initrd_img_end = .;
  . = ALIGN(0x1000);
  _data_start = .;
  .data : { *(.data*) }
  _data_end = .;

  . = 0x0C000000;
  
  . = ALIGN(0x1000);
  _bss_start = .;
  .bss : { *(.bss*) }
  _bss_end = .;

    
  . = ALIGN(0x1000);
  _freearea_start = .;
  _freearea_end = 0x2E000000;

  _intr_sp =   0x2F000000;
  _stack_end = 0x30000000; /* 512MB */
}
