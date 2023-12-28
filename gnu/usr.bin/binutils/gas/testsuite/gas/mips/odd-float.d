#objdump: -dr --prefix-addresses
#name: MIPS odd float
#as: -32 -march=sb1 -EL --fatal-warnings

.*: +file format .*mips.*

Disassembly of section .text:
0+00 <[^>]*> lwxc1	\$f1,a0\(a1\)
0+04 <[^>]*> swxc1	\$f3,a0\(a1\)
#pass
