#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DWARF-2 location information with branch swapping disassembly
#as: -32
#source: loc-swap.s

# Check branch swapping with DWARF-2 location information (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0c90      	move	a0,s0
[0-9a-f]+ <[^>]*> 45a4      	jrc	a0
[0-9a-f]+ <[^>]*> 4584      	jr	a0
[0-9a-f]+ <[^>]*> 0ff0      	move	ra,s0
[0-9a-f]+ <[^>]*> 459f      	jr	ra
[0-9a-f]+ <[^>]*> 0c90      	move	a0,s0
[0-9a-f]+ <[^>]*> 0ff0      	move	ra,s0
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
[0-9a-f]+ <[^>]*> 0c90      	move	a0,s0
[0-9a-f]+ <[^>]*> 45c4      	jalr	a0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0ff0      	move	ra,s0
[0-9a-f]+ <[^>]*> 45c4      	jalr	a0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0c90      	move	a0,s0
[0-9a-f]+ <[^>]*> f400 0000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0ff0      	move	ra,s0
[0-9a-f]+ <[^>]*> f400 0000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
