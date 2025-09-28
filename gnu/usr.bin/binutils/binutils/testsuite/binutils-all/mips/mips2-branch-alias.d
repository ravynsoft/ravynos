#PROG: objcopy
#objdump: -m mips:6000 -d --prefix-addresses --show-raw-insn
#name: MIPS2 branch instruction alias disassembly
#source: mips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10000000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04010000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04110000 	bal	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04100000 	bltzal	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 10200000 	beqz	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 14200000 	bnez	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 50200000 	beqzl	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 54200000 	bnezl	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
