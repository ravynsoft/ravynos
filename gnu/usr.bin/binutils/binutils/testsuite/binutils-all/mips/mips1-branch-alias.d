#PROG: objcopy
#objdump: -m mips:3000 -d --prefix-addresses --show-raw-insn
#name: MIPS1 branch instruction alias disassembly
#source: mips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04010000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04110000 	bal	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04100000 	bltzal	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 10200000 	beqz	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 14200000 	bnez	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 50200000 	.word	0x50200000
[0-9a-f]+ <[^>]*> 54200000 	.word	0x54200000
	\.\.\.
