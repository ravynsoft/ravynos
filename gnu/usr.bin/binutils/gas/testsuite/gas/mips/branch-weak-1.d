#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch to a weak symbol
#as: -32 --defsym align=12
#source: branch-weak.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1000ffff 	b	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
