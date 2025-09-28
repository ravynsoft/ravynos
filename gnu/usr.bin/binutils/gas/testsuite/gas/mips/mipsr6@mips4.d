#objdump: -dr --prefix-addresses
#name: MIPS mips4 non-fp
#source: mips4.s

# Test mips4 *non-fp* insturctions.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> pref	0x4,0\(a0\)
	...
