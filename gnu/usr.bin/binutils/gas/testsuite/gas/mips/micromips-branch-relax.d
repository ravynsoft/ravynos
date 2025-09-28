#objdump: -dr --show-raw-insn
#name: Relax microMIPS branches
#as: -mips32r2 -32 -relax-branch
#warning_output: micromips-branch-relax.l
#source: micromips-branch-relax.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <test>:
[ 0-9a-f]+:	9400 fffe 	b	[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9403 fffe 	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	b403 fffe 	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	cfff      	b	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40e0 fffe 	bc	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4060 fffe 	bal	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	4260 fffe 	bals	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	8dff      	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	adff      	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	cfff      	b	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	8dff      	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	adff      	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9400 fffe 	b	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40e0 fffe 	bc	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4060 fffe 	bal	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	4260 fffe 	bals	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9403 fffe 	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	b403 fffe 	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	cfff      	b	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	8dff      	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	adff      	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9400 fffe 	b	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40e0 fffe 	bc	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4060 fffe 	bal	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	4260 fffe 	bals	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9403 fffe 	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	b403 fffe 	bnez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	7400 0000 	jals	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	b483 fffe 	bne	v1,a0,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	9483 fffe 	beq	v1,a0,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4043 fffe 	bgez	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4003 fffe 	bltz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40d4 fffe 	bgtz	s4,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4094 fffe 	blez	s4,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40a3 fffe 	bnezc	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	40e3 fffe 	beqzc	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	401e fffe 	bltz	s8,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	405e fffe 	bgez	s8,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1950 	addu	v1,a0,a1
[ 0-9a-f]+:	401e fffe 	bltz	s8,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	7400 0000 	jals	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	405e fffe 	bgez	s8,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	7400 0000 	jals	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	43a0 fffe 	bc1t	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4380 fffe 	bc1f	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	42a0 fffe 	bc2t	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	4280 fffe 	bc2f	[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
[ 0-9a-f]+:	b483 fffe 	bne	v1,a0,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	b403 fffe 	bnez	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0083 0b50 	slt	at,v1,a0
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0083 0b50 	slt	at,v1,a0
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0083 0b90 	sltu	at,v1,a0
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0083 0b90 	sltu	at,v1,a0
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	4003 fffe 	bltz	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	4003 fffe 	bltz	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0064 0b50 	slt	at,a0,v1
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0064 0b50 	slt	at,a0,v1
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0064 0b90 	sltu	at,a0,v1
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0064 0b90 	sltu	at,a0,v1
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	4083 fffe 	blez	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0064 0b50 	slt	at,a0,v1
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0064 0b50 	slt	at,a0,v1
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0064 0b90 	sltu	at,a0,v1
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0064 0b90 	sltu	at,a0,v1
[ 0-9a-f]+:	b401 fffe 	bnez	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	40c3 fffe 	bgtz	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0083 0b50 	slt	at,v1,a0
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0083 0b50 	slt	at,v1,a0
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0083 0b90 	sltu	at,v1,a0
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	0083 0b90 	sltu	at,v1,a0
[ 0-9a-f]+:	9401 fffe 	beqz	at,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	4043 fffe 	bgez	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	4043 fffe 	bgez	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	9483 fffe 	beq	v1,a0,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	9403 fffe 	beqz	v1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1
[ 0-9a-f]+:	9403 fffe 	beqz	v1,[0-9a-f]+ <.*\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	d400 0000 	j	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	test3
[ 0-9a-f]+:	00a4 1b10 	xor	v1,a0,a1

[0-9a-f]+ <.*>:
	\.\.\.

[0-9a-f]+ <test2>:
	\.\.\.

[0-9a-f]+ <test3>:
[ 0-9a-f]+:	05d8      	addu	v1,a0,a1
	\.\.\.
