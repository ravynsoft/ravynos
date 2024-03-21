#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS CACHE instruction
#as: -32

# Check MIPS CACHE instruction assembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> bc4507ff 	cache	0x5,2047\(v0\)
[0-9a-f]+ <[^>]*> bc65f800 	cache	0x5,-2048\(v1\)
[0-9a-f]+ <[^>]*> bc850800 	cache	0x5,2048\(a0\)
[0-9a-f]+ <[^>]*> bca5f7ff 	cache	0x5,-2049\(a1\)
[0-9a-f]+ <[^>]*> bcc57fff 	cache	0x5,32767\(a2\)
[0-9a-f]+ <[^>]*> bce58000 	cache	0x5,-32768\(a3\)
[0-9a-f]+ <[^>]*> 3c010001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 00280821 	addu	at,at,t0
[0-9a-f]+ <[^>]*> bc258000 	cache	0x5,-32768\(at\)
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 00290821 	addu	at,at,t1
[0-9a-f]+ <[^>]*> bc257fff 	cache	0x5,32767\(at\)
[0-9a-f]+ <[^>]*> 3c010001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 002a0821 	addu	at,at,t2
[0-9a-f]+ <[^>]*> bc259000 	cache	0x5,-28672\(at\)
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 002b0821 	addu	at,at,t3
[0-9a-f]+ <[^>]*> bc256fff 	cache	0x5,28671\(at\)
	\.\.\.
