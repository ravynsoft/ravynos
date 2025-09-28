#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch to absolute expression (n32)
#as: -n32 -march=from-abi
#source: mips16-branch-absolute.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 1000 	b	00001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> f000 6000 	bteqz	00001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> f000 6100 	btnez	0000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> f000 2200 	beqz	v0,00001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
