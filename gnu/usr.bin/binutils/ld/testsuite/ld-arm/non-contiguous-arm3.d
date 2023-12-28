#name: non-contiguous-arm3
#source: non-contiguous-arm.s
#ld: --enable-non-contiguous-regions -T non-contiguous-arm3.ld -z max-page-size=0x10000
#objdump: -rdth
#xfail: [is_generic]
#skip: arm*nacl

.*:     file format elf32-(little|big)arm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.raml         00000018  1fff0000  1fff0000  00010000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.ramu         00000010  20000000  1fff0018  00020000  2\*\*3
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 \.ramz         00000050  30040000  20000010  00030000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  3 .ARM.attributes 00000012  00000000  00000000  00030050  2\*\*0
                  CONTENTS, READONLY
SYMBOL TABLE:
1fff0000 l    d  .raml	00000000 .raml
20000000 l    d  .ramu	00000000 .ramu
30040000 l    d  .ramz	00000000 .ramz
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 l    df \*ABS\*	00000000 .*non-contiguous-arm.o
20000008 l     F .ramu	00000008 __code4_veneer
1fff0018 g       .raml	00000000 _raml_end
20000000 g       .ramu	00000000 _ramu_start
1fff000c g     F .raml	00000000 code2
30040000 g       .ramz	00000000 _ramz_start
1fff0000 g       .raml	00000000 _raml_start
20000000 g     F .ramu	00000000 code3
1fff0000 g     F .raml	00000000 code1
30040050 g       .ramz	00000000 _ramz_end
30040000 g     F .ramz	00000000 code4
20000010 g       .ramu	00000000 _ramu_end


Disassembly of section .raml:

1fff0000 \<code1\>:
1fff0000:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0004:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0008:	ebffffff 	bl	1fff000c \<code2\>

1fff000c \<code2\>:
1fff000c:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0010:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0014:	eb003ff9 	bl	20000000 \<code3\>

Disassembly of section .ramu:

20000000 \<code3\>:
20000000:	e1a00000 	nop			@ \(mov r0, r0\)
20000004:	ebffffff 	bl	20000008 \<__code4_veneer\>

20000008 \<__code4_veneer\>:
20000008:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 2000000c \<__code4_veneer\+0x4\>
2000000c:	30040000 	.word	0x30040000

Disassembly of section .ramz:

30040000 \<code4\>:
30040000:	e1a00000 	.word	0xe1a00000
30040004:	e1a00000 	.word	0xe1a00000
30040008:	e1a00000 	.word	0xe1a00000
3004000c:	e1a00000 	.word	0xe1a00000
30040010:	e1a00000 	.word	0xe1a00000
30040014:	e1a00000 	.word	0xe1a00000
30040018:	e1a00000 	.word	0xe1a00000
3004001c:	e1a00000 	.word	0xe1a00000
30040020:	e1a00000 	.word	0xe1a00000
30040024:	e1a00000 	.word	0xe1a00000
30040028:	e1a00000 	.word	0xe1a00000
3004002c:	e1a00000 	.word	0xe1a00000
30040030:	e1a00000 	.word	0xe1a00000
30040034:	e1a00000 	.word	0xe1a00000
30040038:	e1a00000 	.word	0xe1a00000
3004003c:	e1a00000 	.word	0xe1a00000
30040040:	e1a00000 	.word	0xe1a00000
30040044:	e1a00000 	.word	0xe1a00000
30040048:	e1a00000 	.word	0xe1a00000
3004004c:	e1a00000 	.word	0xe1a00000
