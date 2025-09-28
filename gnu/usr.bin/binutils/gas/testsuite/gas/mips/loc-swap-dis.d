#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DWARF-2 location information with branch swapping disassembly
#as: -32
#source: loc-swap.s

# Check branch swapping with DWARF-2 location information.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 02002025 	move	a0,s0
[0-9a-f]+ <[^>]*> 00800008 	jr	a0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 00800008 	jr	a0
[0-9a-f]+ <[^>]*> 0200f825 	move	ra,s0
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 02002025 	move	a0,s0
[0-9a-f]+ <[^>]*> 0200f825 	move	ra,s0
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 02002025 	move	a0,s0
[0-9a-f]+ <[^>]*> 0080f809 	jalr	a0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0200f825 	move	ra,s0
[0-9a-f]+ <[^>]*> 0080f809 	jalr	a0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0c000000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_26	bar
[0-9a-f]+ <[^>]*> 02002025 	move	a0,s0
[0-9a-f]+ <[^>]*> 0200f825 	move	ra,s0
[0-9a-f]+ <[^>]*> 0c000000 	jal	0+0000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_26	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
