#PROG: objcopy
#objdump: -M no-aliases -d --prefix-addresses --show-raw-insn
#name: microMIPS branch canonical alias disassembly
#source: micromips-branch-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 9400 0000 	beq	zero,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4040 0000 	bgez	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 40e0 0000 	beqzc	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 9401 0000 	beq	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> b401 0000 	bne	at,zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4060 0000 	bgezal	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4260 0000 	bgezals	zero,[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 0000 0000 	sll	zero,zero,0x0
	\.\.\.
