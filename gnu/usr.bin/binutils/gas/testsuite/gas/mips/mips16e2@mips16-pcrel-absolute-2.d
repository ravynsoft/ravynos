#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression 2
#as: -32
#source: mips16-pcrel-absolute-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[0-9a-f]+ <[^>]*> f222 fd54 	daddiu	v0,4660
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[0-9a-f]+ <[^>]*> f222 3a54 	ld	v0,4660\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
