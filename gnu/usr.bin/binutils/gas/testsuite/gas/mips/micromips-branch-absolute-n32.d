#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to absolute expression (n32)
#as: -n32 -march=from-abi
#source: micromips-branch-absolute.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	00001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 0000 	bltzal	zero,00001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 0000 	beqzc	v0,00001018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 40a2 0000 	bnezc	v0,0000101c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\*ABS\*\+0x1231
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
