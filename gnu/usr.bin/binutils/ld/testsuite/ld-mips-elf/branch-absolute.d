#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS link branch to absolute expression
#source: ../../../gas/testsuite/gas/mips/branch-absolute.s
#ld: -Ttext 0 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 1000008c 	b	0+001234 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0411008a 	bal	0+001234 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04100088 	bltzal	zero,0+001234 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10400086 	beqz	v0,0+001234 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 14400084 	bnez	v0,0+001234 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
