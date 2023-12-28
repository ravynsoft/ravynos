#PROG: objcopy
#objdump: -M no-aliases -m mips:6000 -d --prefix-addresses --show-raw-insn
#name: MIPS2 branch canonical alias disassembly
#source: mips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10000000 	beq	zero,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04010000 	bgez	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04110000 	bgezal	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 04100000 	bltzal	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 10200000 	beq	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 14200000 	bne	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 50200000 	beql	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 54200000 	bnel	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 00000000 	sll	zero,zero,0x0
	\.\.\.
