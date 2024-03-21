#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch to an external symbol
#as: -32 -KPIC
#source: branch-extern.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1000ffff 	b	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
