#name: .inst pseudo-opcode test 1 - big endian version
#as: -mbig-endian
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince
#source: inst-po.s

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <.text> 11a01009 	movne	r1, r9
00000004 <.text\+0x4> bf0c      	ite	eq
00000006 <.text\+0x6> 4649      	moveq	r1, r9
00000008 <.text\+0x8> 4649      	movne	r1, r9
0000000a <.text\+0xa> 0000      	.short	0x0000
0000000c <.text\+0xc> 1234      	.short	0x1234
0000000e <.text\+0xe> bf0c      	ite	eq
00000010 <.text\+0x10> 4649      	moveq	r1, r9
00000012 <.text\+0x12> 4649      	movne	r1, r9
00000014 <.text\+0x14> 4649      	mov	r1, r9
00000016 <.text\+0x16> ea4f 0109 	mov.w	r1, r9
0000001a <.text\+0x1a> ea4f 0109 	mov.w	r1, r9
0000001e <.text\+0x1e> bf00      	nop
