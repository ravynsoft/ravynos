#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 jump delay slot scheduling for EXTEND instructions
#as: -32

# Verify that EXTEND instructions are not scheduled into a jump delay slot.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> eb40      	jalr	v1
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
