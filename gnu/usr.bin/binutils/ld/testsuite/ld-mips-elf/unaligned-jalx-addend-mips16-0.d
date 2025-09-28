#name: MIPS16 JALX to unaligned symbol with addend 0
#source: unaligned-jalx-addend-0.s -mips16
#source: unaligned-insn.s
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1c18 000d 	jalx	1c000034 <bar2>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1c18 000c 	jalx	1c000030 <bar0>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1c18 000c 	jalx	1c000030 <bar0>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1c18 000e 	jalx	1c000038 <bar4>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
