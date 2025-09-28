#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-5pic-64
#source: branch-misc-5.s
#as: -64 -call_shared

# Test branches to undefined symbols and a defined local symbol
# in another section (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	0+0004 <g6\+0x4>
			0: R_MICROMIPS_PC16_S1	x1\-0x4
			0: R_MIPS_NONE	\*ABS\*\-0x4
			0: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	0+0008 <g6\+0x8>
			4: R_MICROMIPS_PC16_S1	x2\-0x4
			4: R_MIPS_NONE	\*ABS\*\-0x4
			4: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	0+000c <g6\+0xc>
			8: R_MICROMIPS_PC16_S1	\.Ldata\-0x4
			8: R_MIPS_NONE	\*ABS\*\-0x4
			8: R_MIPS_NONE	\*ABS\*\-0x4
	\.\.\.
