#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-5pic
#source: branch-misc-5.s
#as: -32 -call_shared

# Test branches to undefined symbols and a defined local symbol
# in another section.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 1000ffff 	b	00000000 <g6>
[ 	]*0: R_MIPS_PC16	x1
0+0004 <[^>]*> 00000000 	nop
0+0008 <[^>]*> 1000ffff 	b	00000008 <g6\+0x8>
[ 	]*8: R_MIPS_PC16	x2
0+000c <[^>]*> 00000000 	nop
0+0010 <[^>]*> 1000ffff 	b	00000010 <g6\+0x10>
[ 	]*10: R_MIPS_PC16	\.Ldata
0+0014 <[^>]*> 00000000 	nop
	\.\.\.
