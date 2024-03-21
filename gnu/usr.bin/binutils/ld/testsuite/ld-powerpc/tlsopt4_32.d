#source: tlsopt4_32.s
#source: tlslib32.s
#as: -a32
#ld: 
#objdump: -dr
#target: powerpc*-*-*

.*

Disassembly of section \.text:

0+1800094 <__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.opt1:

0+1800098 <\.opt1>:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(2c 04 00 00|00 00 04 2c) 	cmpwi   r4,0
.*:	(41 82 00 0c|0c 00 82 41) 	beq     .*
.*:	(38 62 90 10|10 90 62 38) 	addi    r3,r2,-28656
.*:	(48 00 00 08|08 00 00 48) 	b       .*
.*:	(38 62 90 10|10 90 62 38) 	addi    r3,r2,-28656

Disassembly of section \.opt2:

0+18000b0 <\.opt2>:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(2c 04 00 00|00 00 04 2c) 	cmpwi   r4,0
.*:	(41 82 00 08|08 00 82 41) 	beq     .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(38 62 90 10|10 90 62 38) 	addi    r3,r2,-28656

Disassembly of section \.opt3:

0+18000c4 <\.opt3>:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(48 00 00 0c|0c 00 00 48) 	b       .*
.*:	(38 62 90 10|10 90 62 38) 	addi    r3,r2,-28656
.*:	(48 00 00 08|08 00 00 48) 	b       .*
.*:	(38 62 90 08|08 90 62 38) 	addi    r3,r2,-28664
#pass
