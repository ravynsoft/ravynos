#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-2pic-64
#source: branch-misc-2.s
#as: -64 -call_shared

# Test branches to global symbols in current file (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
	\.\.\.
	\.\.\.
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0040 <x\+0x4>
			3c: R_MICROMIPS_PC16_S1	g1\-0x4
			3c: R_MIPS_NONE	\*ABS\*\-0x4
			3c: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0048 <x\+0xc>
			44: R_MICROMIPS_PC16_S1	g2\-0x4
			44: R_MIPS_NONE	\*ABS\*\-0x4
			44: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0050 <x\+0x14>
			4c: R_MICROMIPS_PC16_S1	g3\-0x4
			4c: R_MIPS_NONE	\*ABS\*\-0x4
			4c: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0058 <x\+0x1c>
			54: R_MICROMIPS_PC16_S1	g4\-0x4
			54: R_MIPS_NONE	\*ABS\*\-0x4
			54: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0060 <x\+0x24>
			5c: R_MICROMIPS_PC16_S1	g5\-0x4
			5c: R_MIPS_NONE	\*ABS\*\-0x4
			5c: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0+0068 <x\+0x2c>
			64: R_MICROMIPS_PC16_S1	g6\-0x4
			64: R_MIPS_NONE	\*ABS\*\-0x4
			64: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
