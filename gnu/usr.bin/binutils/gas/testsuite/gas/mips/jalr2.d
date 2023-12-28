#name: MIPS JAL with an addend
#as: -mips2 -32 -KPIC
#objdump: -dr

.*


Disassembly of section \.text:

.* <test>:
.*:	afbc0010 	sw	gp,16\(sp\)
.*:	8f990000 	lw	t9,0\(gp\)
.*: R_MIPS_GOT16	\.text
.*:	2739004c 	addiu	t9,t9,76
.*: R_MIPS_LO16	\.text
.*:	0320f809 	jalr	t9
.*: R_MIPS_JALR	local
.*:	00000000 	nop
.*:	8fbc0010 	lw	gp,16\(sp\)
.*:	8f990000 	lw	t9,0\(gp\)
.*: R_MIPS_GOT16	\.text
.*:	27390058 	addiu	t9,t9,88
.*: R_MIPS_LO16	\.text
# No R_MIPS_JALR here, because the target address had an addend.
.*:	0320f809 	jalr	t9
.*:	00000000 	nop
.*:	8fbc0010 	lw	gp,16\(sp\)
.*:	8f990000 	lw	t9,0\(gp\)
.*: R_MIPS_CALL16	global
.*:	0320f809 	jalr	t9
.*: R_MIPS_JALR	global
.*:	00000000 	nop
.*:	8fbc0010 	lw	gp,16\(sp\)
.*:	8f99000c 	lw	t9,12\(gp\)
.*: R_MIPS_CALL16	global
# No R_MIPS_JALR here either, for the same reason.
.*:	0320f809 	jalr	t9
.*:	00000000 	nop
.*:	8fbc0010 	lw	gp,16\(sp\)

.* <local>:
	\.\.\.
