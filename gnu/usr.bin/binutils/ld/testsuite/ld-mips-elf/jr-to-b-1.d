#name: jr to b conversion
#source: jr-to-b-1.s
#as: -mips1
#ld: -shared
#objdump: -d

.*:     file format .*


Disassembly of section \.text:

.* <s>:
.*:	03e00008 	jr	ra
.*:	24020001 	li	v0,1

.* <t>:
.*:	3c1c.... 	lui	gp,.*
.*:	279c.... 	addiu	gp,gp,.*
.*:	0399e021 	addu	gp,gp,t9
.*:	8f998018 	lw	t9,.*\(gp\)
.*:	00000000 	nop
.*:	2739.... 	addiu	t9,t9,.*
.*:	1000fff7 	b	.* <s>
.*:	00000000 	nop
#pass
