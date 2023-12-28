#objdump: -dr --prefix-addresses
#as: -mabi=o64
#name: MIPS III load $zero
#source: ld-zero-3.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lwu	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> ld	zero,22136\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x1234
[0-9a-f]+ <[^>]*> ori	at,at,0x5600
[0-9a-f]+ <[^>]*> addu	at,at,v0
[0-9a-f]+ <[^>]*> lld	zero,120\(at\)
	\.\.\.
