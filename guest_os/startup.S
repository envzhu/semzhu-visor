.section .text , "ax"
.global startup_entry
startup_entry:
  # ldr    x0, = el2_vector
  # msr    vbar_el1, x0
  ldr    x0, = _kernel_sp
  mov    sp, x0
  
  bl  main

cpu_hlt:
  msr  daifset, #0b1111   /* Disable Interrupt */
  wfe
  b   cpu_hlt
  