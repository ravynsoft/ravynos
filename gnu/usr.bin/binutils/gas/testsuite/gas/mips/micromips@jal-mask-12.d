#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0xaaaaaaa0
#name: MIPS jal mask 1.2
#as: -32
#source: jal-mask-1.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
aaaaaaa0 <[^>]*> d400 0000 	j	a8000000 <[^>]*>
aaaaaaa4 <[^>]*> 0c00      	nop
aaaaaaa6 <[^>]*> d555 5552 	j	aaaaaaa4 <[^>]*>
aaaaaaaa <[^>]*> 0c00      	nop
aaaaaaac <[^>]*> d6aa aaac 	j	ad555558 <[^>]*>
aaaaaab0 <[^>]*> 0c00      	nop
aaaaaab2 <[^>]*> d7ff fffe 	j	affffffc <[^>]*>
aaaaaab6 <[^>]*> 0c00      	nop
aaaaaab8 <[^>]*> f400 0000 	jal	a8000000 <[^>]*>
aaaaaabc <[^>]*> 0000 0000 	nop
aaaaaac0 <[^>]*> f555 5552 	jal	aaaaaaa4 <[^>]*>
aaaaaac4 <[^>]*> 0000 0000 	nop
aaaaaac8 <[^>]*> f6aa aaac 	jal	ad555558 <[^>]*>
aaaaaacc <[^>]*> 0000 0000 	nop
aaaaaad0 <[^>]*> f7ff fffe 	jal	affffffc <[^>]*>
aaaaaad4 <[^>]*> 0000 0000 	nop
	\.\.\.
