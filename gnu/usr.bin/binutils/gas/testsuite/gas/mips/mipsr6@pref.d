#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PREF instruction
#as: -32 --defsym tpref=1
#source: cache.s

# Check MIPS PREF instruction assembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7c457fb5 	pref	0x5,255\(v0\)
[0-9a-f]+ <[^>]*> 7c658035 	pref	0x5,-256\(v1\)
	\.\.\.
