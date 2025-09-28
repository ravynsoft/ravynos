#objdump: -dr --prefix-addresses
#as: -32
#name: MIPS II load $zero
#source: ld-zero-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> ori	at,at,0x5000
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> ll	zero,1656\(at\)
	\.\.\.
