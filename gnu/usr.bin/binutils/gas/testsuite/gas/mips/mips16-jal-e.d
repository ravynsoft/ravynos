#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 explicit extended JAL instructions
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
