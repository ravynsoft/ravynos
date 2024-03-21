# name: Data disassembler test (no symbols)
# skip: *-*-pe *-*-wince
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
00000000 <.text> 20010000 	.word	0x20010000
00000004 <.text\+0x4> 000000f9 	.word	0x000000f9
00000008 <.text\+0x8> 00004cd5 	.word	0x00004cd5
