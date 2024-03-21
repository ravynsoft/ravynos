#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-5pic-64
#source: branch-misc-5.s
#as: -64 -call_shared

# Test branches to undefined symbols and a defined local symbol
# in another section (MIPSr6).

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 10000000 	b	0000000000000004 <g6\+0x4>
[ 	]*0: R_MIPS_PC16	x1-0x4
[ 	]*0: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*0: R_MIPS_NONE	\*ABS\*-0x4
0+0004 <[^>]*> 00000000 	nop
0+0008 <[^>]*> 10000000 	b	000000000000000c <g6\+0xc>
[ 	]*8: R_MIPS_PC16	x2-0x4
[ 	]*8: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*8: R_MIPS_NONE	\*ABS\*-0x4
0+000c <[^>]*> 00000000 	nop
0+0010 <[^>]*> 10000000 	b	0000000000000014 <g6\+0x14>
[ 	]*10: R_MIPS_PC16	\.Ldata-0x4
[ 	]*10: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*10: R_MIPS_NONE	\*ABS\*-0x4
0+0014 <[^>]*> 00000000 	nop
	\.\.\.
