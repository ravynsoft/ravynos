#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-2pic-64
#source: branch-misc-2.s
#as: -64 -call_shared

# Test branches to global symbols in current file.

.*: +file format .*mips.*

Disassembly of section .text:
	\.\.\.
	\.\.\.
	\.\.\.
0+003c <[^>]*> 04110000 	bal	0000000000000040 <x\+0x4>
[ 	]*3c: R_MIPS_PC16	g1-0x4
[ 	]*3c: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*3c: R_MIPS_NONE	\*ABS\*-0x4
0+0040 <[^>]*> 00000000 	nop
0+0044 <[^>]*> 04110000 	bal	0000000000000048 <x\+0xc>
[ 	]*44: R_MIPS_PC16	g2-0x4
[ 	]*44: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*44: R_MIPS_NONE	\*ABS\*-0x4
0+0048 <[^>]*> 00000000 	nop
0+004c <[^>]*> 04110000 	bal	0000000000000050 <x\+0x14>
[ 	]*4c: R_MIPS_PC16	g3-0x4
[ 	]*4c: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*4c: R_MIPS_NONE	\*ABS\*-0x4
0+0050 <[^>]*> 00000000 	nop
0+0054 <[^>]*> 04110000 	bal	0000000000000058 <x\+0x1c>
[ 	]*54: R_MIPS_PC16	g4-0x4
[ 	]*54: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*54: R_MIPS_NONE	\*ABS\*-0x4
0+0058 <[^>]*> 00000000 	nop
0+005c <[^>]*> 04110000 	bal	0000000000000060 <x\+0x24>
[ 	]*5c: R_MIPS_PC16	g5-0x4
[ 	]*5c: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*5c: R_MIPS_NONE	\*ABS\*-0x4
0+0060 <[^>]*> 00000000 	nop
0+0064 <[^>]*> 04110000 	bal	0000000000000068 <x\+0x2c>
[ 	]*64: R_MIPS_PC16	g6-0x4
[ 	]*64: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*64: R_MIPS_NONE	\*ABS\*-0x4
0+0068 <[^>]*> 00000000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
