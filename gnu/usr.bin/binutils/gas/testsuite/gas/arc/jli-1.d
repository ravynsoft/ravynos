#as:
#objdump: -Dr

.*: +file format .*


Disassembly of section .group:

00000000 <jlitab.foo>:
   0:	0[10] 00 00 0[01]             	.word	0x00000001
   4:	0[60] 00 00 0[06]             	.word	0x00000006
   8:	0[70] 00 00 0[07]             	.word	0x00000007

Disassembly of section .text:

00000000 <.text>:
   0:	5800                	jli_s	0
			0: R_ARC_JLI_SECTOFF	__jli.foo

Disassembly of section .jlitab:

00000000 <__jli.foo>:
   0:	0001 0000           	b	0	;0 <foo>
			0: R_ARC_S25H_PCREL	foo

#...
