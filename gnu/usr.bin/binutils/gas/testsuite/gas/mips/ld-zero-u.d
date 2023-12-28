#objdump: -dr --prefix-addresses
#as: -32
#name: microMIPS load $zero

.*: +file format .*mips.*
Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> ori	at,at,0x5000
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lwp	zero,1656\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> ori	at,at,0x5000
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> ldp	zero,1656\(at\)
	\.\.\.
