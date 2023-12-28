#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch to absolute expression with addend
#source: ../../../gas/testsuite/gas/mips/mips16-branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f445 1014 	b	0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> f445 6012 	bteqz	0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> f445 6110 	btnez	0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> f445 220e 	beqz	v0,0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> f445 2a0c 	bnez	v0,0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
