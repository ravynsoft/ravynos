#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-2pic
#source: branch-misc-2.s
#as: -32 -call_shared

# Test branches to global symbols in current file.

.*: +file format .*mips.*

Disassembly of section .text:
	\.\.\.
	\.\.\.
	\.\.\.
0+003c <[^>]*> 0411ffff 	bal	0000003c <x>
[ 	]*3c: R_MIPS_PC16	g1
0+0040 <[^>]*> 00000000 	nop
0+0044 <[^>]*> 0411ffff 	bal	00000044 <x\+0x8>
[ 	]*44: R_MIPS_PC16	g2
0+0048 <[^>]*> 00000000 	nop
0+004c <[^>]*> 0411ffff 	bal	0000004c <x\+0x10>
[ 	]*4c: R_MIPS_PC16	g3
0+0050 <[^>]*> 00000000 	nop
0+0054 <[^>]*> 0411ffff 	bal	00000054 <x\+0x18>
[ 	]*54: R_MIPS_PC16	g4
0+0058 <[^>]*> 00000000 	nop
0+005c <[^>]*> 0411ffff 	bal	0000005c <x\+0x20>
[ 	]*5c: R_MIPS_PC16	g5
0+0060 <[^>]*> 00000000 	nop
0+0064 <[^>]*> 0411ffff 	bal	00000064 <x\+0x28>
[ 	]*64: R_MIPS_PC16	g6
0+0068 <[^>]*> 00000000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
