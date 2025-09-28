#as: -EL -I$srcdir/$subdir -mfpu=neon --generate-missing-build-notes=no
#objdump: --syms --special-syms -d
#name: ARM Mapping Symbols for miscellaneous directives
# This test is only valid on EABI based ports.
#target: *-*-*eabi* *-*-linux-* *-*-elf *-*-nacl*
#source: mapmisc.s


.*: +file format .*arm.*

SYMBOL TABLE:
0+00 l    d  .text	00000000 .text
0+00 l    d  .data	00000000 .data
0+00 l    d  .bss	00000000 .bss
0+00 l     F .text	00000000 foo
0+00 l       .text	00000000 \$a
0+04 l       .text	00000000 \$d
0+08 l       .text	00000000 \$a
0+0c l       .text	00000000 \$d
0+10 l       .text	00000000 \$a
0+14 l       .text	00000000 \$d
0+18 l       .text	00000000 \$a
0+1c l       .text	00000000 \$d
0+20 l       .text	00000000 \$a
0+24 l       .text	00000000 \$d
0+28 l       .text	00000000 \$a
0+2c l       .text	00000000 \$d
0+34 l       .text	00000000 \$a
0+38 l       .text	00000000 \$d
0+48 l       .text	00000000 \$a
0+4c l       .text	00000000 \$d
0+50 l       .text	00000000 \$a
0+54 l       .text	00000000 \$d
0+58 l       .text	00000000 \$a
0+5c l       .text	00000000 \$d
0+64 l       .text	00000000 \$a
0+68 l       .text	00000000 \$d
0+70 l       .text	00000000 \$a
0+74 l       .text	00000000 \$d
0+84 l       .text	00000000 \$a
0+88 l       .text	00000000 \$d
0+8c l       .text	00000000 \$a
0+90 l       .text	00000000 \$d
0+94 l       .text	00000000 \$a
0+98 l       .text	00000000 \$d
0+9c l       .text	00000000 \$a
0+a0 l       .text	00000000 \$d
0+a4 l       .text	00000000 \$a
0+a8 l       .text	00000000 \$a
0+b0 l       .text	00000000 string
0+b0 l       .text	00000000 \$d
0+b4 l       .text	00000000 \$d
0+b3 l       .text	00000000 \$d
0+00 l    d  .ARM.attributes	00000000 .ARM.attributes



Disassembly of section .text:

00000000 <foo>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)
   4:	64636261 	.word	0x64636261
   8:	e1a00000 	nop			@ \(mov r0, r0\)
   c:	00636261 	.word	0x00636261
  10:	e1a00000 	nop			@ \(mov r0, r0\)
  14:	00676665 	.word	0x00676665
  18:	e1a00000 	nop			@ \(mov r0, r0\)
  1c:	006a6968 	.word	0x006a6968
  20:	e1a00000 	nop			@ \(mov r0, r0\)
  24:	0000006b 	.word	0x0000006b
  28:	e1a00000 	nop			@ \(mov r0, r0\)
  2c:	0000006c 	.word	0x0000006c
  30:	00000000 	.word	0x00000000
  34:	e1a00000 	nop			@ \(mov r0, r0\)
  38:	0000006d 	.word	0x0000006d
	...
  48:	e1a00000 	nop			@ \(mov r0, r0\)
  4c:	3fc00000 	.word	0x3fc00000
  50:	e1a00000 	nop			@ \(mov r0, r0\)
  54:	40200000 	.word	0x40200000
  58:	e1a00000 	nop			@ \(mov r0, r0\)
  5c:	00000000 	.word	0x00000000
  60:	400c0000 	.word	0x400c0000
  64:	e1a00000 	nop			@ \(mov r0, r0\)
  68:	00000000 	.word	0x00000000
  6c:	40120000 	.word	0x40120000
  70:	e1a00000 	nop			@ \(mov r0, r0\)
  74:	00000004 	.word	0x00000004
  78:	00000004 	.word	0x00000004
  7c:	00000004 	.word	0x00000004
  80:	00000004 	.word	0x00000004
  84:	e1a00000 	nop			@ \(mov r0, r0\)
  88:	00000000 	.word	0x00000000
  8c:	e1a00000 	nop			@ \(mov r0, r0\)
  90:	00000000 	.word	0x00000000
  94:	e1a00000 	nop			@ \(mov r0, r0\)
  98:	00000000 	.word	0x00000000
  9c:	e1a00000 	nop			@ \(mov r0, r0\)
  a0:	7778797a 	.word	0x7778797a
  a4:	e1a00000 	nop			@ \(mov r0, r0\)
  a8:	e1a00000 	nop			@ \(mov r0, r0\)
  ac:	e51f0000 	ldr	r0, \[pc, #-0\]	@ b4 <string\+0x4>
000000b0 <string>:
  b0:	6261      	.short	0x6261
  b2:	63          	.byte	0x63
  b3:	00          	.byte	0x00
  b4:	000000b0 	.word	0x000000b0
