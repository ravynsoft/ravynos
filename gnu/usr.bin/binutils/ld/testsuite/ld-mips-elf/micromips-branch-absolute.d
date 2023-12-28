#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS link branch to absolute expression
#source: ../../../gas/testsuite/gas/mips/micromips-branch-absolute.s
#ld: -Ttext 0 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 0118 	bc	0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 4060 0116 	bal	0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 0112 	bltzal	zero,0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 010e 	beqzc	v0,0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 40a2 010c 	bnezc	v0,0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
