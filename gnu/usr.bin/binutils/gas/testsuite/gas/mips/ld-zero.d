#objdump: -dr --prefix-addresses
#as: -32
#name: MIPS load $zero

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lb	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lbu	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lh	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lhu	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lw	zero,22136\(at\)
	\.\.\.
