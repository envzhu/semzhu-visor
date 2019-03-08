OUTPUT_ARCH(elf64-littleaarch64)
ENTRY(startup_entry)

MEMORY
{ 

	ramall(rwx)	: o = 0x00000000, l =  0x40000000 /* 1GB */
	hyp_ram(rwx): o = 0x00000000, l =  0x10000000 /* 256MB */
	/* guest_ram(rwx):o= 0x00000000, l =  0x10000000 /* 256MB */
  mmio(rw)		: o = 0x3F000000, l =	 0x01000000 /* 16MB */
}

SECTIONS
{
  .text : {
    . = ALIGN(0x1000);
    *(.text.boot)
    *(.text.vector)
    *(.text*)
  } > hyp_ram

  .os : {
  . = ALIGN(0x1000);

    _rodata_start = .;
      *(.rodata*)
    _rodata_end = .;
    . = ALIGN(0x1000);
    _sampleos_img_start = .;
       sampleOS-img.o(.data)
    _sampleos_img_end = .;
    
    . = ALIGN(0x1000);
    _linux_img_start = .;
      linux-img.o(.data)
    _linux_img_end = .;
  } > hyp_ram

  . = ALIGN(0x1000);
  .data : {
    _data_start = .;
      *(.data*)
    _data_end = .;
  } > hyp_ram

  .bss : {
    _bss_start = .;
      *(.bss*)
    _bss_end = .;
  } > hyp_ram

  _intr_sp =   0x1E000000;
  _stack_end = 0x20000000; /* 512MB */
}
