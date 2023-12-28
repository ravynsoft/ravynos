#PROG: objcopy
#objdump: -d --prefix-addresses --show-raw-insn
#name: microMIPS branch instruction alias disassembly
#source: micromips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 9400 0000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4040 0000 	b	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 40e0 0000 	bc	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 9401 0000 	beqz	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> b401 0000 	bnez	at,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4060 0000 	bal	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4260 0000 	bals	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
