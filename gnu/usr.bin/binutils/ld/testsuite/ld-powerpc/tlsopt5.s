 .globl _start
 .weak aaaaa
_start:
 .cfi_startproc
 addi 3,2,gd@got@tlsgd
 bl __tls_get_addr(gd@tlsgd)
 nop
 bl aaaaa
 nop
 .cfi_endproc
