#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch to absolute expression (n32)
#as: -n32 -march=from-abi
#source: branch-absolute.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 10000000 	b	00001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x1230
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04110000 	bal	0000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x1230
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04100000 	bltzal	zero,00001014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x1230
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,0000101c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x1230
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 14400000 	bnez	v0,00001024 <foo\+0x24>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x1230
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
