#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch to absolute expression with addend (n64)
#as: -64 -march=from-abi
#source: mips16-branch-absolute-addend.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 1000 	b	0000000000001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> f000 6000 	bteqz	0000000000001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> f000 6100 	btnez	000000000000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> f000 2200 	beqz	v0,0000000000001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,0000000000001014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
