#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-4
#as: -32

# Verify PC-relative relocations do not overflow.

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
([0-9a-f]+) <[^>]*> 1000ffff 	b	\1 <foo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
([0-9a-f]+) <[^>]*> 1000ffff 	b	\1 <\.Lfoo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\.Lbar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.

Disassembly of section \.init:
([0-9a-f]+) <[^>]*> 1000ffff 	b	\1 <bar>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo
[0-9a-f]+ <[^>]*> 00000000 	nop
([0-9a-f]+) <[^>]*> 1000ffff 	b	\1 <\.Lbar>
[ 	]*[0-9a-f]+: R_MIPS_PC16	\.Lfoo
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
