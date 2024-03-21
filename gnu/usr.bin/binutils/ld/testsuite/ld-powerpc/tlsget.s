 .text
 .abiversion 2
 .globl _start
_start:
 .cfi_startproc
 pla 3,gd@got@tlsgd@pcrel
 bl __tls_get_addr@notoc(gd@tlsgd)

 addis 3,2,gd@got@tlsgd@ha
 addi 3,3,gd@got@tlsgd@l
 bl __tls_get_addr(gd@tlsgd)
 nop

 bl fun@notoc

 bl fun
 nop

 .type fun,@gnu_indirect_function
fun:
 pla 3,1f@pcrel
1:
 blr
.cfi_endproc
