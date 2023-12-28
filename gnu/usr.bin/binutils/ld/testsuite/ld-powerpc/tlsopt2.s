 .section ".no_opt2", "ax", %progbits
# this section should not be optimised since we have old-style
# __tls_get_addr without marker relocs, and two arg setup insns
# feed into one __tls_get_addr call.
 addi 3,2,gd@got@tlsgd
 cmpdi 4,0
 beq 0f
 addi 3,2,gd@got@tlsgd
0:
 bl __tls_get_addr
 nop
