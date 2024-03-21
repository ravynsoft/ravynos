#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS CACHE instruction
#source: cache.s
#as: -32

# Check MIPS CACHE instruction assembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7c457fa5 	cache	0x5,255\(v0\)
[0-9a-f]+ <[^>]*> 7c658025 	cache	0x5,-256\(v1\)
	\.\.\.
