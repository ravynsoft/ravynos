#as: -EL --generate-missing-build-notes=no
#objdump: --syms --special-syms -d
#name: ARM Mapping Symbols with multiple sections
# This test is only valid on EABI based ports.
#target: *-*-*eabi* *-*-linux-* *-*-elf *-*-nacl*
#source: mapsecs.s


.*: +file format .*arm.*

SYMBOL TABLE:
0+00 l    d  .text	00000000 .text
0+00 l    d  .data	00000000 .data
0+00 l    d  .bss	00000000 .bss
0+00 l    d  .text.f1	00000000 .text.f1
0+00 l     F .text.f1	00000000 f1
0+00 l       .text.f1	00000000 \$a
0+08 l       .text.f1	00000000 f1a
0+00 l    d  .text.f2	00000000 .text.f2
0+00 l     F .text.f2	00000000 f2
0+00 l       .text.f2	00000000 \$a
0+04 l       .text.f2	00000000 \$d
0+08 l       .text.f2	00000000 f2a
0+08 l       .text.f2	00000000 \$a
0+00 l    d  .ARM.attributes	00000000 .ARM.attributes



Disassembly of section .text.f1:

00000000 <f1>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)
   4:	e1a00000 	nop			@ \(mov r0, r0\)

00000008 <f1a>:
   8:	e1a00000 	nop			@ \(mov r0, r0\)

Disassembly of section .text.f2:

00000000 <f2>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)
   4:	00000001 	.word	0x00000001

00000008 <f2a>:
   8:	e1a00000 	nop			@ \(mov r0, r0\)
