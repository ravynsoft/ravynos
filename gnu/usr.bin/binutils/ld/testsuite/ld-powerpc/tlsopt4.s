 .section ".tbss","awT",@nobits
 .global gd0
 .align 3
gd0: .space 8

 .section ".opt1", "ax", %progbits
 addi 3,2,gd@got@tlsgd
 cmpdi 4,0
 beq 0f
 bl __tls_get_addr(gd@tlsgd)
 nop
 b 1f
0:
 bl __tls_get_addr(gd@tlsgd)
 nop
1:

 .section ".opt2", "ax", %progbits
 addi 3,2,gd@got@tlsgd
 cmpdi 4,0
 beq 0f
 addi 3,2,gd@got@tlsgd
0:
 bl __tls_get_addr(gd@tlsgd)
 nop

 .section ".opt3", "ax", %progbits
 addi 3,2,gd@got@tlsgd
 b 0f
 addi 3,2,gd0@got@tlsgd
 b 1f
0:
 bl __tls_get_addr(gd@tlsgd)
 nop
 b 2f
1:
 bl __tls_get_addr(gd0@tlsgd)
 nop
2:
