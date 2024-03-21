#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch 2
#source: ../../../gas/testsuite/gas/mips/mips16-branch-reloc-2.s
#ld: -Ttext 0x1c000000 -e bar

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 1016 	b	1c001030 <bar>
[0-9a-f]+ <[^>]*> f000 6014 	bteqz	1c001030 <bar>
[0-9a-f]+ <[^>]*> f000 6112 	btnez	1c001030 <bar>
[0-9a-f]+ <[^>]*> f000 2210 	beqz	v0,1c001030 <bar>
[0-9a-f]+ <[^>]*> f000 2a0e 	bnez	v0,1c001030 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
