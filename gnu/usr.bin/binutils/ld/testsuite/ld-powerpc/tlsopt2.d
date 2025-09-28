#source: tlsopt2.s
#source: tlslib.s
#as: -a64
#ld: 
#objdump: -dr
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

0+100000e8 <\.__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.no_opt2:

0+100000ec <\.no_opt2>:
.*:	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*:	(2c 24 00 00|00 00 24 2c) 	cmpdi   r4,0
.*:	(41 82 00 08|08 00 82 41) 	beq     .*
.*:	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*:	(4b ff ff ed|ed ff ff 4b) 	bl      100000e8 <\.__tls_get_addr>
.*:	(60 00 00 00|00 00 00 60) 	nop
