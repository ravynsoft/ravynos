# name: Data disassembler test (function symbol)
# skip: *-*-pe *-*-wince
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
00000000 <main> 20010000 	.word	0x20010000
00000004 <main\+0x4> 000000f9 	.word	0x000000f9
00000008 <main\+0x8> 00004cd5 	.word	0x00004cd5
