#source: tlsopt3_32.s
#source: tlslib32.s
#as: -a32
#ld: 
#objdump: -dr
#target: powerpc*-*-*

.*

Disassembly of section \.text:

0+1800094 <__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.no_opt3:

0+1800098 <\.no_opt3>:
.*:	(38 7e ff ec|ec ff 7e 38) 	addi    r3,r30,-20
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(38 7e ff f4|f4 ff 7e 38) 	addi    r3,r30,-12
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(4b ff ff ed|ed ff ff 4b) 	bl      1800094 <__tls_get_addr>
.*:	(48 00 00 08|08 00 00 48) 	b       .*
.*:	(4b ff ff e5|e5 ff ff 4b) 	bl      1800094 <__tls_get_addr>
#pass
