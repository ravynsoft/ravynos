 .text
 .globl _start
_start:
 .cfi_startproc
 addi 3,2,gd@got@tlsgd
 bl __tls_get_addr_desc(gd@tlsgd)
 nop
 .cfi_endproc

 .section .text.pad1,"ax"
 .space 0x1ffff90

 .section .text.far,"ax"
 .cfi_startproc
 addi 3,2,gd@got@tlsgd
 bl __tls_get_addr_desc(gd@tlsgd)
 nop
 .cfi_endproc
