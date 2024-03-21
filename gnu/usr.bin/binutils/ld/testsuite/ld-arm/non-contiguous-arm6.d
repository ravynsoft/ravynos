#name: non-contiguous-arm6
#source: non-contiguous-arm.s
#ld: --enable-non-contiguous-regions -T non-contiguous-arm6.ld -z max-page-size=0x10000
#objdump: -rdth
#xfail: [is_generic]
#skip: arm*nacl

.*:     file format elf32-(little|big)arm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.raml         00000028  1fff0000  1fff0000  00010000  2\*\*3
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.ramz         00000050  40040000  30000000  00020000  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .ARM.attributes 00000012  00000000  00000000  00020050  2\*\*0
                  CONTENTS, READONLY
SYMBOL TABLE:
1fff0000 l    d  .raml	00000000 .raml
40040000 l    d  .ramz	00000000 .ramz
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 l    df \*ABS\*	00000000 .*non-contiguous-arm.o
1fff0020 l     F .raml	00000008 __code4_veneer
1fff0028 g       .raml	00000000 _raml_end
30000000 g       .raml	00000000 _ramu_start
1fff000c g     F .raml	00000000 code2
40040000 g       .ramz	00000000 _ramz_start
1fff0000 g       .raml	00000000 _raml_start
1fff0018 g     F .raml	00000000 code3
1fff0000 g     F .raml	00000000 code1
40040050 g       .ramz	00000000 _ramz_end
40040000 g     F .ramz	00000000 code4
30000000 g       .raml	00000000 _ramu_end

Disassembly of section .raml:

1fff0000 \<code1\>:
1fff0000:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0004:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0008:	ebffffff 	bl	1fff000c \<code2\>

1fff000c \<code2\>:
1fff000c:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0010:	e1a00000 	nop			@ \(mov r0, r0\)
1fff0014:	ebffffff 	bl	1fff0018 \<code3\>

1fff0018 \<code3\>:
1fff0018:	e1a00000 	nop			@ \(mov r0, r0\)
1fff001c:	ebffffff 	bl	1fff0020 \<__code4_veneer\>

1fff0020 \<__code4_veneer\>:
1fff0020:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 1fff0024 \<__code4_veneer\+0x4\>
1fff0024:	40040000 	.word	0x40040000

Disassembly of section .ramz:

40040000 \<code4\>:
40040000:	e1a00000 	.word	0xe1a00000
40040004:	e1a00000 	.word	0xe1a00000
40040008:	e1a00000 	.word	0xe1a00000
4004000c:	e1a00000 	.word	0xe1a00000
40040010:	e1a00000 	.word	0xe1a00000
40040014:	e1a00000 	.word	0xe1a00000
40040018:	e1a00000 	.word	0xe1a00000
4004001c:	e1a00000 	.word	0xe1a00000
40040020:	e1a00000 	.word	0xe1a00000
40040024:	e1a00000 	.word	0xe1a00000
40040028:	e1a00000 	.word	0xe1a00000
4004002c:	e1a00000 	.word	0xe1a00000
40040030:	e1a00000 	.word	0xe1a00000
40040034:	e1a00000 	.word	0xe1a00000
40040038:	e1a00000 	.word	0xe1a00000
4004003c:	e1a00000 	.word	0xe1a00000
40040040:	e1a00000 	.word	0xe1a00000
40040044:	e1a00000 	.word	0xe1a00000
40040048:	e1a00000 	.word	0xe1a00000
4004004c:	e1a00000 	.word	0xe1a00000
