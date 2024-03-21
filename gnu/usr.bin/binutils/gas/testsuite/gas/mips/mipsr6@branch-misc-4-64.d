#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-4-64
#as: -64
#source: branch-misc-4.s

# Verify PC-relative relocations do not overflow.

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <\.Lfoo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\.Lbar-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.

Disassembly of section \.init:
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <bar\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\-0x4
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <\.Lbar\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\.Lfoo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
