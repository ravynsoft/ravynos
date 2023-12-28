#objdump: -dr --prefix-addresses --show-raw-insn -mmips:micromips
#name: microMIPS instruction size 1
#as: -32 -march=mips64 -mmicromips
#source: micromips-size-1.s
#warning_output: micromips-size-1.l

# Test microMIPS instruction size overrides (#1).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0544      	addu	v0,v0,a0
[0-9a-f]+ <[^>]*> 0544      	addu	v0,v0,a0
[0-9a-f]+ <[^>]*> 0082 1150 	addu	v0,v0,a0
[0-9a-f]+ <[^>]*> 01cc 6150 	addu	t4,t4,t6
[0-9a-f]+ <[^>]*> 01cc 6150 	addu	t4,t4,t6
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 45c4      	jalr	a0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45c4      	jalr	a0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03e4 0f3c 	jalr	a0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45d8      	jalr	t8
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45d8      	jalr	t8
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03f8 0f3c 	jalr	t8
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45c5      	jalr	a1
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45c5      	jalr	a1
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03e5 0f3c 	jalr	a1
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45d9      	jalr	t9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45d9      	jalr	t9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03f9 0f3c 	jalr	t9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03da 0f3c 	jalr	s8,k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 03da 0f3c 	jalr	s8,k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+0084 <.*\+0x84>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> cfff      	b	0+008a <.*\+0x8a>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+008e <.*\+0x8e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9407 fffe 	beqz	a3,0+0094 <.*\+0x94>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8fff      	beqz	a3,0+009a <.*\+0x9a>
			9a: R_MICROMIPS_PC7_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9407 fffe 	beqz	a3,0+009e <.*\+0x9e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 941b fffe 	beqz	k1,0+00a4 <.*\+0xa4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 941b fffe 	beqz	k1,0+00aa <.*\+0xaa>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00b0 <.*\+0xb0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0230 8150 	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00b8 <.*\+0xb8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0410      	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00be <.*\+0xbe>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0230 8150 	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+00c6 <.*\+0xc6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0410      	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+00cc <.*\+0xcc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0410      	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+00d2 <.*\+0xd2>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0230 8150 	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00da <.*\+0xda>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00e2 <.*\+0xe2>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+00ea <.*\+0xea>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+00f2 <.*\+0xf2>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+00fa <.*\+0xfa>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,0+0100 <.*\+0x100>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+0106 <.*\+0x106>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0+010c <.*\+0x10c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+0112 <.*\+0x112>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0118 <.*\+0x118>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0230 8150 	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+0120 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0126 <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0410      	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+012c <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0132 <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0230 8150 	addu	s0,s0,s1
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+013a <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0140 <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+0148 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+014e <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 5482 1230 	add\.ps	\$f2,\$f2,\$f4
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+0156 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+015c <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 4042 fffe 	bgez	v0,0+0162 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0168 <.*\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 4c81      	addiu	sp,sp,256
[0-9a-f]+ <[^>]*> 253a      	sll	v0,v1,5
[0-9a-f]+ <[^>]*> 253a      	sll	v0,v1,5
[0-9a-f]+ <[^>]*> 0043 2800 	sll	v0,v1,0x5
[0-9a-f]+ <[^>]*> 0043 6800 	sll	v0,v1,0xd
[0-9a-f]+ <[^>]*> 0043 6800 	sll	v0,v1,0xd
[0-9a-f]+ <[^>]*> 014b 2800 	sll	t2,t3,0x5
[0-9a-f]+ <[^>]*> 014b 2800 	sll	t2,t3,0x5
[0-9a-f]+ <[^>]*> 5843 2800 	dsll	v0,v1,0x5
[0-9a-f]+ <[^>]*> 5843 2808 	dsll32	v0,v1,0x5
[0-9a-f]+ <[^>]*> 5843 2808 	dsll32	v0,v1,0x5
[0-9a-f]+ <[^>]*> 5843 6800 	dsll	v0,v1,0xd
[0-9a-f]+ <[^>]*> 5843 6808 	dsll32	v0,v1,0xd
[0-9a-f]+ <[^>]*> 5843 6808 	dsll32	v0,v1,0xd
[0-9a-f]+ <[^>]*> 594b 2800 	dsll	t2,t3,0x5
[0-9a-f]+ <[^>]*> 594b 2808 	dsll32	t2,t3,0x5
[0-9a-f]+ <[^>]*> 594b 2808 	dsll32	t2,t3,0x5
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
