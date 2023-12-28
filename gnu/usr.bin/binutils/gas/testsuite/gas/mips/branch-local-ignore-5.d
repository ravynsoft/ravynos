#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 5 (ignore branch ISA)
#as: -32 -mignore-branch-isa
#source: branch-local-5.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> e80f      	not	s0
[0-9a-f]+ <[^>]*> 17f6      	b	0+001000 <foo>
[0-9a-f]+ <[^>]*> e80f      	not	s0
[0-9a-f]+ <[^>]*> 22f4      	beqz	v0,0+001000 <foo>
[0-9a-f]+ <[^>]*> e80f      	not	s0
[0-9a-f]+ <[^>]*> 60f2      	bteqz	0+001000 <foo>
[0-9a-f]+ <[^>]*> e80f      	not	s0
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> e80f      	not	s0
	\.\.\.
