 .section ".tbss","awT",@nobits
 .global gd0
 .align 3
gd0: .space 8

 .section ".opt1", "ax", %progbits
 addi 3,30,gd@got@tlsgd
 cmpwi 4,0
 beq 0f
 bl __tls_get_addr(gd@tlsgd)
 b 1f
0:
 bl __tls_get_addr(gd@tlsgd)
1:

 .section ".opt2", "ax", %progbits
 addi 3,30,gd@got@tlsgd
 cmpwi 4,0
 beq 0f
 addi 3,30,gd@got@tlsgd
0:
 bl __tls_get_addr(gd@tlsgd)

 .section ".opt3", "ax", %progbits
 addi 3,30,gd@got@tlsgd
 b 0f
 addi 3,30,gd0@got@tlsgd
 b 1f
0:
 bl __tls_get_addr(gd@tlsgd)
 b 2f
1:
 bl __tls_get_addr(gd0@tlsgd)
2:
