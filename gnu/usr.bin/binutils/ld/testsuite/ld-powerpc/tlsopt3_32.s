 .section ".tbss","awT",@nobits
 .global gd0
 .align 3
gd0: .space 8

 .section ".no_opt3", "ax", %progbits
# this section should also not be optimised
 addi 3,30,gd@got@tlsgd
 b 0f
 addi 3,30,gd0@got@tlsgd
 b 1f
0:
 bl __tls_get_addr
 b 2f
1:
 bl __tls_get_addr
2:
