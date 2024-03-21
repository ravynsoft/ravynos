#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed branch to an external symbol
#as: -32 -KPIC -mips1 --relax-branch
#source: branch-extern.s
#warning_output: branch-extern.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 8f810000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 24210000 	addiu	at,at,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	bar
[0-9a-f]+ <[^>]*> 00200008 	jr	at
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
