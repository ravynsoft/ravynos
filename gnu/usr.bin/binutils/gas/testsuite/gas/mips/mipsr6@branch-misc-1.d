#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-1
#as: -32
#source: branch-misc-1.s

# Test the branches to local symbols in current file.

.*: +file format .*mips.*

Disassembly of section .text:
	\.\.\.
	\.\.\.
	\.\.\.
0+003c <[^>]*> 0411ffff 	bal	0000003c <[^>]*>
[	]*3c: .*R_MIPS_PC16	l1
0+0040 <[^>]*> 00000000 	nop
0+0044 <[^>]*> 0411ffff 	bal	00000044 <[^>]*>
[	]*44: .*R_MIPS_PC16	l2
0+0048 <[^>]*> 00000000 	nop
0+004c <[^>]*> 0411ffff 	bal	0000004c <[^>]*>
[	]*4c: .*R_MIPS_PC16	l3
0+0050 <[^>]*> 00000000 	nop
0+0054 <[^>]*> 0411ffff 	bal	00000054 <[^>]*>
[	]*54: .*R_MIPS_PC16	l4
0+0058 <[^>]*> 00000000 	nop
0+005c <[^>]*> 0411ffff 	bal	0000005c <[^>]*>
[	]*5c: .*R_MIPS_PC16	l5
0+0060 <[^>]*> 00000000 	nop
0+0064 <[^>]*> 0411ffff 	bal	00000064 <[^>]*>
[	]*64: .*R_MIPS_PC16	l6
0+0068 <[^>]*> 00000000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
