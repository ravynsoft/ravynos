#PROG: objcopy
#objdump: -m mips:isa32r6 -d --prefix-addresses --show-raw-insn
#name: MIPS32r6 branch instruction alias disassembly
#source: mips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04010000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04110000 	bal	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04100000 	nal
[0-9a-f]+ <[^>]*> 10200000 	beqz	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 14200000 	bnez	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 50200000 	.word	0x50200000
[0-9a-f]+ <[^>]*> 54200000 	.word	0x54200000
	\.\.\.
