#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS bge
#source: bge.s
#as: -32

# Test the bge macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0004 <text_label\+0x4>
			4: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 4044 fffe 	bgez	a0,0+0008 <text_label\+0x8>
			8: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4085 fffe 	blez	a1,0+000e <text_label\+0xe>
			e: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4044 fffe 	bgez	a0,0+0014 <text_label\+0x14>
			14: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 40c4 fffe 	bgtz	a0,0+001a <text_label\+0x1a>
			1a: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9024 0002 	slti	at,a0,2
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0024 <text_label\+0x24>
			24: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 0024 0b50 	slt	at,a0,at
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0030 <text_label\+0x30>
			30: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 9024 8000 	slti	at,a0,-32768
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0038 <text_label\+0x38>
			38: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 0024 0b50 	slt	at,a0,at
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0044 <text_label\+0x44>
			44: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 0024 0b50 	slt	at,a0,at
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0054 <text_label\+0x54>
			54: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+005c <text_label\+0x5c>
			5c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 40c4 fffe 	bgtz	a0,0+0060 <text_label\+0x60>
			60: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4005 fffe 	bltz	a1,0+0066 <text_label\+0x66>
			66: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 40c4 fffe 	bgtz	a0,0+006c <text_label\+0x6c>
			6c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0076 <text_label\+0x76>
			76: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+007e <text_label\+0x7e>
			7e: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
