#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed branch to a different section
#as: -32 --relax-branch
#source: branch-section.s
#warning_output: branch-section.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_26	\.init
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.

Disassembly of section \.init:
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
