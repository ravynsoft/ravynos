#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0x55555550
#name: MIPS jal mask 1.1
#as: -32
#source: jal-mask-1.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
55555550 <[^>]*> 08000000 	j	50000000 <[^>]*>
55555554 <[^>]*> 00000000 	nop
55555558 <[^>]*> 0aaaaaa9 	j	5aaaaaa4 <[^>]*>
5555555c <[^>]*> 00000000 	nop
55555560 <[^>]*> 09555556 	j	55555558 <[^>]*>
55555564 <[^>]*> 00000000 	nop
55555568 <[^>]*> 0bffffff 	j	5ffffffc <[^>]*>
5555556c <[^>]*> 00000000 	nop
55555570 <[^>]*> 0c000000 	jal	50000000 <[^>]*>
55555574 <[^>]*> 00000000 	nop
55555578 <[^>]*> 0eaaaaa9 	jal	5aaaaaa4 <[^>]*>
5555557c <[^>]*> 00000000 	nop
55555580 <[^>]*> 0d555556 	jal	55555558 <[^>]*>
55555584 <[^>]*> 00000000 	nop
55555588 <[^>]*> 0fffffff 	jal	5ffffffc <[^>]*>
5555558c <[^>]*> 00000000 	nop
	\.\.\.
