  .global MY_BUF
  .section  .bss.MY_BUF,"aw",%nobits
  .type	MY_BUF, %object
  .size	MY_BUF, 102400
MY_BUF:
  .space  102400

  .section  .text.foo,"ax",%progbits
  .global foo
  .type	foo, %function
foo:
  ldr r0, .L3
  bx lr
.L3:
  .word	MY_BUF
  .size	foo, .-foo
