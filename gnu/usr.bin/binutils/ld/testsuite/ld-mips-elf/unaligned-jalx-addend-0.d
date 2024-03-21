#name: MIPS JALX to unaligned symbol with addend 0
#source: unaligned-jalx-addend-0.s
#source: unaligned-insn.s -mips16
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7700000d 	jalx	1c000034 <bar2>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 7700000c 	jalx	1c000030 <bar0>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 7700000c 	jalx	1c000030 <bar0>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 7700000e 	jalx	1c000038 <bar4>
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
