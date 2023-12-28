#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to absolute expression with addend (n64)
#as: -64 -march=from-abi
#source: micromips-branch-absolute-addend.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	0000000000001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000000000001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 0000 	bltzal	zero,0000000000001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 0000 	beqzc	v0,0000000000001018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 40a2 0000 	bnezc	v0,000000000000101c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x123468a9
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
