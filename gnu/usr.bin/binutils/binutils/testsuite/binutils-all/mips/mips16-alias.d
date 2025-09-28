#PROG: objcopy
#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 instruction alias disassembly
#as: -mips3

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 0a08      	la	v0,00000020 <bar>
[0-9a-f]+ <[^>]*> b207      	lw	v0,00000020 <bar>
[0-9a-f]+ <[^>]*> fe47      	dla	v0,00000020 <bar>
[0-9a-f]+ <[^>]*> fc43      	ld	v0,00000020 <bar>
	\.\.\.
	\.\.\.
