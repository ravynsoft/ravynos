#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS bltu
#source: bltu.s
#as: -32

# Test the bltu macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0004 <text_label\+0x4>
			4: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 40a5 fffe 	bnezc	a1,0+0008 <text_label\+0x8>
			8: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 40e4 fffe 	beqzc	a0,0+000c <text_label\+0xc>
			c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> b024 0002 	sltiu	at,a0,2
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0014 <text_label\+0x14>
			14: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 0024 0b90 	sltu	at,a0,at
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0020 <text_label\+0x20>
			20: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> b024 8000 	sltiu	at,a0,-32768
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0028 <text_label\+0x28>
			28: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 0024 0b90 	sltu	at,a0,at
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0034 <text_label\+0x34>
			34: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 0024 0b90 	sltu	at,a0,at
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+0044 <text_label\+0x44>
			44: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+004c <text_label\+0x4c>
			4c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 40e4 fffe 	beqzc	a0,0+0050 <text_label\+0x50>
			50: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 40e4 fffe 	beqzc	a0,0+0054 <text_label\+0x54>
			54: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 00a4 0b90 	sltu	at,a0,a1
[0-9a-f]+ <[^>]*> 40a1 fffe 	bnezc	at,0+005c <text_label\+0x5c>
			5c: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0085 0b90 	sltu	at,a1,a0
[0-9a-f]+ <[^>]*> 40e1 fffe 	beqzc	at,0+0064 <text_label\+0x64>
			64: R_MICROMIPS_PC16_S1	external_label
	\.\.\.
