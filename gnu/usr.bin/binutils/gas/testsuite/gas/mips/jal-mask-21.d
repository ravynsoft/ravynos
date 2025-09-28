#objdump: -dr --prefix-addresses --show-raw-insn --adjust-vma=0x55555550
#name: MIPS jal mask 2.1
#as: -32
#source: jal-mask-2.s

# Check address masks for JAL/J instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
55555550 <[^>]*> d400 0000 	j	50000000 <[^>]*>
55555554 <[^>]*> 0c00      	nop
55555556 <[^>]*> d555 5551 	j	52aaaaa2 <[^>]*>
5555555a <[^>]*> 0c00      	nop
5555555c <[^>]*> d6aa aaaa 	j	55555554 <[^>]*>
55555560 <[^>]*> 0c00      	nop
55555562 <[^>]*> d7ff fffb 	j	57fffff6 <[^>]*>
55555566 <[^>]*> 0c00      	nop
55555568 <[^>]*> f400 0004 	jal	50000008 <[^>]*>
5555556c <[^>]*> 0000 0000 	nop
55555570 <[^>]*> f555 5555 	jal	52aaaaaa <[^>]*>
55555574 <[^>]*> 0000 0000 	nop
55555578 <[^>]*> f6aa aaae 	jal	5555555c <[^>]*>
5555557c <[^>]*> 0000 0000 	nop
55555580 <[^>]*> f7ff ffff 	jal	57fffffe <[^>]*>
55555584 <[^>]*> 0000 0000 	nop
55555588 <[^>]*> 7400 0001 	jals	50000002 <[^>]*>
5555558c <[^>]*> 0c00      	nop
5555558e <[^>]*> 7555 5553 	jals	52aaaaa6 <[^>]*>
55555592 <[^>]*> 0c00      	nop
55555594 <[^>]*> 76aa aaad 	jals	5555555a <[^>]*>
55555598 <[^>]*> 0c00      	nop
5555559a <[^>]*> 77ff ffff 	jals	57fffffe <[^>]*>
5555559e <[^>]*> 0c00      	nop
	\.\.\.
