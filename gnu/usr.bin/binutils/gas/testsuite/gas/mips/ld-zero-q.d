#objdump: -dr --prefix-addresses
#as: -32
#name: MIPS R5900 load $zero

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lq	zero,22136\(at\)
	\.\.\.
