#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch to absolute expression with addend (n64)
#as: -64 -march=from-abi
#source: branch-absolute-addend.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 10000000 	b	0000000000001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04110000 	bal	000000000000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04100000 	bltzal	zero,0000000000001014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,000000000000101c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 14400000 	bnez	v0,0000000000001024 <foo\+0x24>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a8
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
