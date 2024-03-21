#objdump: -dr --prefix-address --show-raw-insn
#as: -32 -I$srcdir/$subdir
#name: MIPS16e 64-bit ISA subset disassembly

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> ecd1      	sew	a0
[0-9a-f]+ <[^>]*> ec51      	zew	a0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
