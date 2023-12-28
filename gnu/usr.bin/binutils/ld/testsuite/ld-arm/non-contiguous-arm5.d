#name: non-contiguous-arm5
#source: non-contiguous-arm.s
#ld: --enable-non-contiguous-regions -T non-contiguous-arm5.ld -z max-page-size=0x10000
#objdump: -rdth
#xfail: [is_generic]

.*:     file format elf32-(little|big)arm.*

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.raml         0000000c  1fff0000  1fff0000  00010000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.ramu         00000014  20000000  1fff000c  00020000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 \.ramz         00000050  20040000  20000014  00030000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  3 .ARM.attributes 00000012  00000000  00000000  .*  2\*\*0
                  CONTENTS, READONLY
SYMBOL TABLE:
1fff0000 l    d  .raml	00000000 .raml
20000000 l    d  .ramu	00000000 .ramu
20040000 l    d  .ramz	00000000 .ramz
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 l    df \*ABS\*	00000000 .*non-contiguous-arm.o
1fff000c g       .raml	00000000 _raml_end
20000000 g       .ramu	00000000 _ramu_start
20000000 g     F .ramu	00000000 code2
20040000 g       .ramz	00000000 _ramz_start
1fff0000 g       .raml	00000000 _raml_start
2000000c g     F .ramu	00000000 code3
1fff0000 g     F .raml	00000000 code1
20040050 g       .ramz	00000000 _ramz_end
20040000 g     F .ramz	00000000 code4
20000014 g       .ramu	00000000 _ramu_end


Disassembly of section .raml:

1fff0000 \<code1\>:
1fff0000:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0004:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0008:	eb003ffc 	bl	20000000 \<code2\>

Disassembly of section .ramu:

20000000 \<code2\>:
20000000:	e1a00000 	nop			@ \(mov r0, r0\)
20000004:	e1a00000 	nop			@ \(mov r0, r0\)
20000008:	ebffffff 	bl	2000000c \<code3\>

2000000c \<code3\>:
2000000c:	e1a00000 	nop			@ \(mov r0, r0\)
20000010:	eb00fffa 	bl	20040000 \<code4\>

Disassembly of section .ramz:

20040000 \<code4\>:
20040000:	e1a00000 	.word	0xe1a00000
20040004:	e1a00000 	.word	0xe1a00000
20040008:	e1a00000 	.word	0xe1a00000
2004000c:	e1a00000 	.word	0xe1a00000
20040010:	e1a00000 	.word	0xe1a00000
20040014:	e1a00000 	.word	0xe1a00000
20040018:	e1a00000 	.word	0xe1a00000
2004001c:	e1a00000 	.word	0xe1a00000
20040020:	e1a00000 	.word	0xe1a00000
20040024:	e1a00000 	.word	0xe1a00000
20040028:	e1a00000 	.word	0xe1a00000
2004002c:	e1a00000 	.word	0xe1a00000
20040030:	e1a00000 	.word	0xe1a00000
20040034:	e1a00000 	.word	0xe1a00000
20040038:	e1a00000 	.word	0xe1a00000
2004003c:	e1a00000 	.word	0xe1a00000
20040040:	e1a00000 	.word	0xe1a00000
20040044:	e1a00000 	.word	0xe1a00000
20040048:	e1a00000 	.word	0xe1a00000
2004004c:	e1a00000 	.word	0xe1a00000
