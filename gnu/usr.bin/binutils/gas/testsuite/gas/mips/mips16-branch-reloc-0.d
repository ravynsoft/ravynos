#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation 0
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 100f      	b	00001020 <bar>
[0-9a-f]+ <[^>]*> 600e      	bteqz	00001020 <bar>
[0-9a-f]+ <[^>]*> 610d      	btnez	00001020 <bar>
[0-9a-f]+ <[^>]*> 220c      	beqz	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> 2a0b      	bnez	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
