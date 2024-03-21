#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch addend 2
#source: ../../../gas/testsuite/gas/mips/mips16-branch-addend-2.s
#ld: -Ttext 0x1c000000 -e bar

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f121 1010 	b	1c002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 600e 	bteqz	1c002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 610c 	btnez	1c002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 220a 	beqz	v0,1c002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 2a08 	bnez	v0,1c002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
