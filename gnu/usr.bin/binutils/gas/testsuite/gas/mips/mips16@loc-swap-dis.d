#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DWARF-2 location information with branch swapping disassembly
#as: -32
#source: loc-swap.s

# Check branch swapping with DWARF-2 location information (MIPS16).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6790      	move	a0,s0
[0-9a-f]+ <[^>]*> ec00      	jr	a0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ec00      	jr	a0
[0-9a-f]+ <[^>]*> 65f8      	move	ra,s0
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6790      	move	a0,s0
[0-9a-f]+ <[^>]*> 65f8      	move	ra,s0
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6790      	move	a0,s0
[0-9a-f]+ <[^>]*> ec40      	jalr	a0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 65f8      	move	ra,s0
[0-9a-f]+ <[^>]*> ec40      	jalr	a0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1800 0000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MIPS16_26	bar
[0-9a-f]+ <[^>]*> 6790      	move	a0,s0
[0-9a-f]+ <[^>]*> 65f8      	move	ra,s0
[0-9a-f]+ <[^>]*> 1800 0000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MIPS16_26	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
