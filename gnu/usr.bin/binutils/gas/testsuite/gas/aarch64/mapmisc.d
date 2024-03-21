#as: -EL -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: --syms --special-syms -d
#name: AArch64 Mapping Symbols for miscellaneous directives
#source: mapmisc.s
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd


.*: +file format .*aarch64.*

SYMBOL TABLE:
0+00 l    d  .text	0+ .text
0+00 l    d  .data	0+ .data
0+00 l    d  .bss	0+ .bss
0+00 l     F .text	0+ foo
0+00 l       .text	0+ \$x
0+04 l       .text	0+ \$d
0+08 l       .text	0+ \$x
0+0c l       .text	0+ \$d
0+10 l       .text	0+ \$x
0+14 l       .text	0+ \$d
0+18 l       .text	0+ \$x
0+1c l       .text	0+ \$d
0+20 l       .text	0+ \$x
0+24 l       .text	0+ \$d
0+28 l       .text	0+ \$x
0+2c l       .text	0+ \$d
0+34 l       .text	0+ \$x
0+38 l       .text	0+ \$d
0+48 l       .text	0+ \$x
0+4c l       .text	0+ \$d
0+50 l       .text	0+ \$x
0+54 l       .text	0+ \$d
0+58 l       .text	0+ \$x
0+5c l       .text	0+ \$d
0+64 l       .text	0+ \$x
0+68 l       .text	0+ \$d
0+70 l       .text	0+ \$x
0+74 l       .text	0+ \$d
0+84 l       .text	0+ \$x
0+88 l       .text	0+ \$d
0+8c l       .text	0+ \$x
0+90 l       .text	0+ \$d
0+94 l       .text	0+ \$x
0+98 l       .text	0+ \$d
0+9c l       .text	0+ \$x
0+a0 l       .text	0+ \$d
0+a4 l       .text	0+ \$x
0+a8 l       .text	0+ \$x



Disassembly of section .text:

0+ <foo>:
   0:	d503201f 	nop
   4:	64636261 	.word	0x64636261
   8:	d503201f 	nop
   c:	00636261 	.word	0x00636261
  10:	d503201f 	nop
  14:	00676665 	.word	0x00676665
  18:	d503201f 	nop
  1c:	006a6968 	.word	0x006a6968
  20:	d503201f 	nop
  24:	0000006b 	.word	0x0000006b
  28:	d503201f 	nop
  2c:	0000006c 	.word	0x0000006c
  30:	00000000 	.word	0x00000000
  34:	d503201f 	nop
  38:	0000006d 	.word	0x0000006d
	...
  48:	d503201f 	nop
  4c:	3fc00000 	.word	0x3fc00000
  50:	d503201f 	nop
  54:	40200000 	.word	0x40200000
  58:	d503201f 	nop
  5c:	00000000 	.word	0x00000000
  60:	400c0000 	.word	0x400c0000
  64:	d503201f 	nop
  68:	00000000 	.word	0x00000000
  6c:	40120000 	.word	0x40120000
  70:	d503201f 	nop
  74:	00000004 	.word	0x00000004
  78:	00000004 	.word	0x00000004
  7c:	00000004 	.word	0x00000004
  80:	00000004 	.word	0x00000004
  84:	d503201f 	nop
  88:	00000000 	.word	0x00000000
  8c:	d503201f 	nop
  90:	00000000 	.word	0x00000000
  94:	d503201f 	nop
  98:	00000000 	.word	0x00000000
  9c:	d503201f 	nop
  a0:	7778797a 	.word	0x7778797a
  a4:	d503201f 	nop
  a8:	d503201f 	nop
