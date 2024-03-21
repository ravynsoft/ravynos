#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0xaaaaaaa0
#name: MIPS jal mask 2.2
#as: -32
#source: jal-mask-2.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
aaaaaaa0 <[^>]*> d400 0000 	j	a8000000 <[^>]*>
aaaaaaa4 <[^>]*> 0c00      	nop
aaaaaaa6 <[^>]*> d555 5551 	j	aaaaaaa2 <[^>]*>
aaaaaaaa <[^>]*> 0c00      	nop
aaaaaaac <[^>]*> d6aa aaaa 	j	ad555554 <[^>]*>
aaaaaab0 <[^>]*> 0c00      	nop
aaaaaab2 <[^>]*> d7ff fffb 	j	affffff6 <[^>]*>
aaaaaab6 <[^>]*> 0c00      	nop
aaaaaab8 <[^>]*> f400 0004 	jal	a8000008 <[^>]*>
aaaaaabc <[^>]*> 0000 0000 	nop
aaaaaac0 <[^>]*> f555 5555 	jal	aaaaaaaa <[^>]*>
aaaaaac4 <[^>]*> 0000 0000 	nop
aaaaaac8 <[^>]*> f6aa aaae 	jal	ad55555c <[^>]*>
aaaaaacc <[^>]*> 0000 0000 	nop
aaaaaad0 <[^>]*> f7ff ffff 	jal	affffffe <[^>]*>
aaaaaad4 <[^>]*> 0000 0000 	nop
aaaaaad8 <[^>]*> 7400 0001 	jals	a8000002 <[^>]*>
aaaaaadc <[^>]*> 0c00      	nop
aaaaaade <[^>]*> 7555 5553 	jals	aaaaaaa6 <[^>]*>
aaaaaae2 <[^>]*> 0c00      	nop
aaaaaae4 <[^>]*> 76aa aaad 	jals	ad55555a <[^>]*>
aaaaaae8 <[^>]*> 0c00      	nop
aaaaaaea <[^>]*> 77ff ffff 	jals	affffffe <[^>]*>
aaaaaaee <[^>]*> 0c00      	nop
	\.\.\.
