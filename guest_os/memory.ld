OUTPUT_ARCH(elf64-littleaarch64)
ENTRY(startup_entry)

SECTIONS
{
  . = 0x0;
  
  .text : { *(.text.boot) *(.text*) }

  .vector : { KEEP(*(.vector)) }

  . = ALIGN(0x1000);
  _rodata_start = .;
  .rodata : { *(.rodata*) }
  _rodata_end = .;
  

  . = ALIGN(0x1000);
  _data_start = .;
  .data : { *(.data*) }
  _data_end = .;

    . = ALIGN(0x1000);
  _bss_start = .;
  .bss : { *(.bss*) }
  _bss_end = .;

  _kernel_sp = 0x10000; /* 64KB */
}
