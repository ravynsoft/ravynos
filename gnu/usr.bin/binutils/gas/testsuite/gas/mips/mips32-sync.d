#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa32
#name: MIPS32 sync instructions
#as: -32 -mips32

.*: +file format .*mips.*

Disassembly of section \.text:
0+0000 <foo> 0000000f[ 	]*sync
0+0004 <foo\+0x4> 0000000f[ 	]*sync
0+0008 <foo\+0x8> 0000004f[ 	]*sync	0x1
0+000c <foo\+0xc> 0000008f[ 	]*sync	0x2
0+0010 <foo\+0x10> 0000078f[ 	]*sync	0x1e
0+0014 <foo\+0x14> 000007cf[ 	]*sync	0x1f
#pass
