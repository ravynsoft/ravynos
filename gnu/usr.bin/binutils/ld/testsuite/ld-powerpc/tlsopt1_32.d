#source: tlsopt1_32.s
#source: tlslib32.s
#as: -a32
#ld: 
#objdump: -dr
#target: powerpc*-*-*

.*

Disassembly of section \.text:

0+1800094 <__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.no_opt1:

0+1800098 <\.no_opt1>:
.*:	(38 7e ff f4|f4 ff 7e 38) 	addi    r3,r30,-12
.*:	(2c 04 00 00|00 00 04 2c) 	cmpwi   r4,0
.*:	(41 82 00 0c|0c 00 82 41) 	beq     .*
.*:	(4b ff ff f1|f1 ff ff 4b) 	bl      1800094 <__tls_get_addr>
.*:	(48 00 00 08|08 00 00 48) 	b       .*
.*:	(4b ff ff e9|e9 ff ff 4b) 	bl      1800094 <__tls_get_addr>
#pass
