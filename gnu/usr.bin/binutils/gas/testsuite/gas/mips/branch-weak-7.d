#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 short branch to a weak symbol
#as: -32 -mips16 --defsym align=4
#source: branch-weak.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f7ff 101e 	b	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
	\.\.\.
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
