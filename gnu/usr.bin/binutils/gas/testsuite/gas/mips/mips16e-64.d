#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#as: -32
#name: MIPS16e-64
#source: mips16e-64.s

# Test the 64bit instructions of mips16e.

.*: +file format .*mips.*

Disassembly of section .text:

[0-9a-f]+ <[^>]*> ecd1      	sew	\$4
[0-9a-f]+ <[^>]*> ec51      	zew	\$4
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
