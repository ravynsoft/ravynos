#PROG: objcopy
#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 unsupported EXTEND and undefined opcode disassembly

# Verify raw hexadecimal EXTEND and inexistent opcode disassembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> f456      	extend	0x456
[0-9a-f]+ <[^>]*> f765      	extend	0x765
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f432      	extend	0x432
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f123      	extend	0x123
[0-9a-f]+ <[^>]*> 6621      	.short	0x6621
[0-9a-f]+ <[^>]*> f456      	extend	0x456
[0-9a-f]+ <[^>]*> e935      	.short	0xe935
[0-9a-f]+ <[^>]*> f765      	extend	0x765
[0-9a-f]+ <[^>]*> ea60      	.short	0xea60
[0-9a-f]+ <[^>]*> f432      	extend	0x432
[0-9a-f]+ <[^>]*> ece0      	.short	0xece0
[0-9a-f]+ <[^>]*> f5aa      	extend	0x5aa
[0-9a-f]+ <[^>]*> e971      	.short	0xe971
[0-9a-f]+ <[^>]*> f655      	extend	0x655
[0-9a-f]+ <[^>]*> ebf1      	.short	0xebf1
[0-9a-f]+ <[^>]*> 6621      	.short	0x6621
[0-9a-f]+ <[^>]*> e935      	.short	0xe935
[0-9a-f]+ <[^>]*> ea60      	.short	0xea60
[0-9a-f]+ <[^>]*> ece0      	.short	0xece0
[0-9a-f]+ <[^>]*> e971      	.short	0xe971
[0-9a-f]+ <[^>]*> ebf1      	.short	0xebf1
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
