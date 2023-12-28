 .section ".no_opt1", "ax", %progbits
# this section should not be optimised since we have old-style
# __tls_get_addr without marker relocs, and the arg setup insn
# is shared with two __tls_get_addr calls.
 addi 3,2,gd@got@tlsgd
 cmpdi 4,0
 beq 0f
 bl __tls_get_addr
 nop
 b 1f
0:
 bl __tls_get_addr
 nop
1:
