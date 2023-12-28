#objdump: -dr --prefix-addresses
#name: MIPS mips4 branch-likely instructions

# Test mips4 branch-likely instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> bc1fl	\$fcc1,0+0000 <text_label>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bc1tl	\$fcc2,0+0000 <text_label>
[0-9a-f]+ <[^>]*> nop
	\.\.\.
