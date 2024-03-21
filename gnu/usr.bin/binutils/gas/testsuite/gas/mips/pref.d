#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PREF instruction
#as: -32 --defsym tpref=1
#source: cache.s

# Check MIPS PREF instruction assembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> cc4507ff 	pref	0x5,2047\(v0\)
[0-9a-f]+ <[^>]*> cc65f800 	pref	0x5,-2048\(v1\)
[0-9a-f]+ <[^>]*> cc850800 	pref	0x5,2048\(a0\)
[0-9a-f]+ <[^>]*> cca5f7ff 	pref	0x5,-2049\(a1\)
[0-9a-f]+ <[^>]*> ccc57fff 	pref	0x5,32767\(a2\)
[0-9a-f]+ <[^>]*> cce58000 	pref	0x5,-32768\(a3\)
[0-9a-f]+ <[^>]*> 3c010001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 00280821 	addu	at,at,t0
[0-9a-f]+ <[^>]*> cc258000 	pref	0x5,-32768\(at\)
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 00290821 	addu	at,at,t1
[0-9a-f]+ <[^>]*> cc257fff 	pref	0x5,32767\(at\)
[0-9a-f]+ <[^>]*> 3c010001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 002a0821 	addu	at,at,t2
[0-9a-f]+ <[^>]*> cc259000 	pref	0x5,-28672\(at\)
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 002b0821 	addu	at,at,t3
[0-9a-f]+ <[^>]*> cc256fff 	pref	0x5,28671\(at\)
	\.\.\.
