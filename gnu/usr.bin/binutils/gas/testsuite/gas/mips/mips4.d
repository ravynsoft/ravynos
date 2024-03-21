#objdump: -dr --prefix-addresses
#name: MIPS mips4 non-fp

# Test mips4 *non-fp* insturctions.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> movn	a0,a2,a2
0+0004 <[^>]*> movz	a0,a2,a2
0+0008 <[^>]*> pref	0x4,0\(a0\)
	...
