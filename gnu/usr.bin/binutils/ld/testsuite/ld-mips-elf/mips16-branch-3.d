#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch 3
#source: mips16-branch.s
#source: ../../../gas/testsuite/gas/mips/mips16-branch-reloc-3.s
#ld: -Ttext 0x1c000000 -e bar

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> f7fe 100e 	b	1c001000 <bar>
[0-9a-f]+ <[^>]*> f7fe 600c 	bteqz	1c001000 <bar>
[0-9a-f]+ <[^>]*> f7fe 610a 	btnez	1c001000 <bar>
[0-9a-f]+ <[^>]*> f7fe 2208 	beqz	v0,1c001000 <bar>
[0-9a-f]+ <[^>]*> f7fe 2a06 	bnez	v0,1c001000 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
