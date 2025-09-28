#name: MIPS JALX to unaligned symbol 0
#source: unaligned-jalx-0.s
#source: unaligned-insn.s -mips16
#as: -EB
#ld: -EB -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 77000008 	jalx	1c000020 <bar0>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 77000009 	jalx	1c000024 <bar2>
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
