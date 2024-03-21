#as: -march=mips1 -32
#objdump: -dz
#name: MIPS coprocessor branches

.*file format .*

Disassembly .*:

0+00 <.*>:
.*	ctc1	a0,c1_fcsr
.*	b	.*
.*	nop
#
.*	ctc1	a0,c1_fcsr
.*	nop
.*	nop
.*	bc1t	.*
.*	nop
#
.*	c\.eq\.s	\$f0,\$f2
.*	b	.*
.*	nop
#
.*	c\.eq\.s	\$f0,\$f2
.*	nop
.*	bc1t	.*
.*	nop
#
.*	ctc1	a0,c1_fcsr
.*	addiu	a1,a1,1
.*	nop
.*	bc1t	.*
.*	nop
#
.*	ctc1	a0,c1_fcsr
.*	addiu	a1,a1,1
.*	addiu	a2,a2,1
.*	bc1t	.*
.*	nop
#
.*	c\.eq\.s	\$f0,\$f2
.*	addiu	a1,a1,1
.*	bc1t	.*
.*	nop
#
.*	ctc1	a0,c1_fcsr
.*	addiu	a1,a1,1
.*	addiu	a2,a2,1
.*	bc1t	.*
.*	addiu	a3,a3,1
#
.*	c\.eq\.s	\$f0,\$f2
.*	addiu	a1,a1,1
.*	bc1t	.*
.*	addiu	a2,a2,1
#
.*	bc1t	.*
.*	addiu	a3,a3,1
#pass
