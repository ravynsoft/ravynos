#name: .inst pseudo-opcode with automatic IT blocks test
#as: -mimplicit-it=always
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <.text> bf08      	it	eq
00000002 <.text\+0x2> 4649      	moveq	r1, r9
00000004 <.text\+0x4> 4649      	mov	r1, r9
00000006 <.text\+0x6> 4649      	mov	r1, r9
00000008 <.text\+0x8> 00001234 	.word	0x00001234
0000000c <.text\+0xc> bf0c      	ite	eq
0000000e <.text\+0xe> 4649      	moveq	r1, r9
00000010 <.text\+0x10> 4649      	movne	r1, r9
