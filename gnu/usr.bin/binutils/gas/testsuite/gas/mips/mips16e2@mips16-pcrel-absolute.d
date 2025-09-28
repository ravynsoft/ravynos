#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression
#as: -32
#source: mips16-pcrel-absolute.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[0-9a-f]+ <[^>]*> f222 4a14 	addiu	v0,4660
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[0-9a-f]+ <[^>]*> f222 9a54 	lw	v0,4660\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
