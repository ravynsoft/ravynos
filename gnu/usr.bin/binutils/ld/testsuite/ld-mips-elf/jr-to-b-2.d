#name: jr not to b conversion
#source: jr-to-b-2.s
#as: -mips1
#ld: -shared
#objdump: -d

.*:     file format .*


Disassembly of section \.text:

.* <s>:
.*:	e820      	jr	ra
.*:	6a01      	li	v0,1

.* <t>:
.*:	3c1c.... 	lui	gp,.*
.*:	279c.... 	addiu	gp,gp,.*
.*:	0399e021 	addu	gp,gp,t9
.*:	8f99.... 	lw	t9,.*\(gp\)
.*:	00000000 	nop
.*:	2739.... 	addiu	t9,t9,.*
.*:	03200008 	jr	t9
.*:	00000000 	nop
#pass
