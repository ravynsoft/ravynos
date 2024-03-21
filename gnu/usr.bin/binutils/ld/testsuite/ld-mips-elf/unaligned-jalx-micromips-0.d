#name: microMIPS JALX to unaligned symbol 0
#source: unaligned-jalx-0.s -mmicromips
#source: unaligned-insn.s
#as: -EB
#ld: -EB -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f300 0008 	jalx	1c000020 <bar0>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> f300 0009 	jalx	1c000024 <bar2>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
