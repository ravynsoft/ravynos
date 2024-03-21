#source: tlsopt3.s
#source: tlslib.s
#as: -a64
#ld: 
#objdump: -dr
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

00000000100000e8 <\.__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.no_opt3:

00000000100000ec <\.no_opt3>:
.*:	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(38 62 80 18|18 80 62 38) 	addi    r3,r2,-32744
.*:	(48 00 00 10|10 00 00 48) 	b       .*
.*:	(4b ff ff ed|ed ff ff 4b) 	bl      100000e8 <\.__tls_get_addr>
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(4b ff ff e1|e1 ff ff 4b) 	bl      100000e8 <\.__tls_get_addr>
.*:	(60 00 00 00|00 00 00 60) 	nop
