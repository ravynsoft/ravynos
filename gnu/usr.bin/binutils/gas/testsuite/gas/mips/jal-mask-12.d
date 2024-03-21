#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0xaaaaaaa0
#name: MIPS jal mask 1.2
#as: -32
#source: jal-mask-1.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
aaaaaaa0 <[^>]*> 08000000 	j	a0000000 <[^>]*>
aaaaaaa4 <[^>]*> 00000000 	nop
aaaaaaa8 <[^>]*> 0aaaaaa9 	j	aaaaaaa4 <[^>]*>
aaaaaaac <[^>]*> 00000000 	nop
aaaaaab0 <[^>]*> 09555556 	j	a5555558 <[^>]*>
aaaaaab4 <[^>]*> 00000000 	nop
aaaaaab8 <[^>]*> 0bffffff 	j	affffffc <[^>]*>
aaaaaabc <[^>]*> 00000000 	nop
aaaaaac0 <[^>]*> 0c000000 	jal	a0000000 <[^>]*>
aaaaaac4 <[^>]*> 00000000 	nop
aaaaaac8 <[^>]*> 0eaaaaa9 	jal	aaaaaaa4 <[^>]*>
aaaaaacc <[^>]*> 00000000 	nop
aaaaaad0 <[^>]*> 0d555556 	jal	a5555558 <[^>]*>
aaaaaad4 <[^>]*> 00000000 	nop
aaaaaad8 <[^>]*> 0fffffff 	jal	affffffc <[^>]*>
aaaaaadc <[^>]*> 00000000 	nop
	\.\.\.
