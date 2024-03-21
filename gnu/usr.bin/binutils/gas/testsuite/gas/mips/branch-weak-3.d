#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed branch to a weak symbol
#as: -32 --relax-branch --defsym align=12
#source: branch-weak.s
#warning_output: branch-weak.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_26	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
