#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation with addend 1
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> f101 1008 	b	00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> f101 6006 	bteqz	00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> f101 6104 	btnez	00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> f101 2202 	beqz	v0,00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> f101 2a00 	bnez	v0,00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
