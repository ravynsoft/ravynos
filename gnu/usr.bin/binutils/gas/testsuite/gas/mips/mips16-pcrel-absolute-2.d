#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression 2
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f222 fd54 	daddiu	v0,4660
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f222 3a54 	ld	v0,4660\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
