#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS local PC-relative relocations 1
#as: -32
#source: pcrel-reloc-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10000007 	b	00000020 <bar>
[0-9a-f]+ <[^>]*> 10000006 	b	00000020 <bar>
[0-9a-f]+ <[^>]*> 10000005 	b	00000020 <bar>
[0-9a-f]+ <[^>]*> 10000004 	b	00000020 <bar>
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
