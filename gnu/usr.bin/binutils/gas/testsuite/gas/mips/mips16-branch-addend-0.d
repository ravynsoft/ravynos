#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation with addend 0
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f121 1010 	b	00002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 600e 	bteqz	00002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 610c 	btnez	00002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 220a 	beqz	v0,00002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f121 2a08 	bnez	v0,00002264 <bar\+0x1234>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
