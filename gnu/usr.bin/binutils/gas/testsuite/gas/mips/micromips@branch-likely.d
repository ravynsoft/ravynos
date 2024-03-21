#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-likely instructions
#source: branch-likely.s
#as: -32

# Check branch-likely instructions (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 9404 fffe 	beqz	a0,0+0000 <text_label>
			0: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> b404 fffe 	bnez	a0,0+0006 <text_label\+0x6>
			6: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9404 fffe 	beqz	a0,0+000c <text_label\+0xc>
			c: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> b404 fffe 	bnez	a0,0+0012 <text_label\+0x12>
			12: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+001c <text_label\+0x1c>
			1c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+0026 <text_label\+0x26>
			26: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+0030 <text_label\+0x30>
			30: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+003a <text_label\+0x3a>
			3a: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+0044 <text_label\+0x44>
			44: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+004e <text_label\+0x4e>
			4e: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+0058 <text_label\+0x58>
			58: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+0062 <text_label\+0x62>
			62: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+006c <text_label\+0x6c>
			6c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+0076 <text_label\+0x76>
			76: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b50 	slt	at,a0,a1
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+0080 <text_label\+0x80>
			80: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b50 	slt	at,a1,a0
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+008a <text_label\+0x8a>
			8a: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+0094 <text_label\+0x94>
			94: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+009e <text_label\+0x9e>
			9e: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> b401 fffe 	bnez	at,0+00a8 <text_label\+0xa8>
			a8: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> 9401 fffe 	beqz	at,0+00b2 <text_label\+0xb2>
			b2: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
