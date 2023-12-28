#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0x55555550
#name: MIPS jal mask 1.1
#as: -32
#source: jal-mask-1.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
55555550 <[^>]*> d400 0000 	j	50000000 <[^>]*>
55555554 <[^>]*> 0c00      	nop
55555556 <[^>]*> d555 5552 	j	52aaaaa4 <[^>]*>
5555555a <[^>]*> 0c00      	nop
5555555c <[^>]*> d6aa aaac 	j	55555558 <[^>]*>
55555560 <[^>]*> 0c00      	nop
55555562 <[^>]*> d7ff fffe 	j	57fffffc <[^>]*>
55555566 <[^>]*> 0c00      	nop
55555568 <[^>]*> f400 0000 	jal	50000000 <[^>]*>
5555556c <[^>]*> 0000 0000 	nop
55555570 <[^>]*> f555 5552 	jal	52aaaaa4 <[^>]*>
55555574 <[^>]*> 0000 0000 	nop
55555578 <[^>]*> f6aa aaac 	jal	55555558 <[^>]*>
5555557c <[^>]*> 0000 0000 	nop
55555580 <[^>]*> f7ff fffe 	jal	57fffffc <[^>]*>
55555584 <[^>]*> 0000 0000 	nop
	\.\.\.
