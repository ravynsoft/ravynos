#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-4-64
#as: -64
#source: branch-misc-4.s

# Verify PC-relative relocations do not overflow (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	[0-9a-f]+ <\.Lfoo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	[0-9a-f]+ <\.Lfoo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.Lbar-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
	\.\.\.

Disassembly of section \.init:
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	[0-9a-f]+ <\.Lbar>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	[0-9a-f]+ <\.Lbar\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.Lfoo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
	\.\.\.
