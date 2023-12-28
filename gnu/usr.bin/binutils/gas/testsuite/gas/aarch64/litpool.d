#objdump: -d
#name: AArch64 Bignums in Literal Pool (PR 16688)
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*: +file format.*aarch64.*

Disassembly of section \.text:

00+00 <.*>:
   0:	9c000080 	ldr	q0, 10 <\.text\+0x10>
   4:	9c0000e1 	ldr	q1, 20 <\.text\+0x20>
   8:	9c000142 	ldr	q2, 30 <\.text\+0x30>
   c:	9c0001a3 	ldr	q3, 40 <\.text\+0x40>
  10:	5a827999 	.word	0x5a827999
  14:	5a827999 	.word	0x5a827999
  18:	5a827999 	.word	0x5a827999
  1c:	5a827999 	.word	0x5a827999
  20:	6ed9eba1 	.word	0x6ed9eba1
  24:	6ed9eba1 	.word	0x6ed9eba1
  28:	6ed9eba1 	.word	0x6ed9eba1
  2c:	6ed9eba1 	.word	0x6ed9eba1
  30:	8f1bbcdc 	.word	0x8f1bbcdc
  34:	8f1bbcdc 	.word	0x8f1bbcdc
  38:	8f1bbcdc 	.word	0x8f1bbcdc
  3c:	8f1bbcdc 	.word	0x8f1bbcdc
  40:	ca62c1d6 	.word	0xca62c1d6
  44:	ca62c1d6 	.word	0xca62c1d6
  48:	ca62c1d6 	.word	0xca62c1d6
  4c:	ca62c1d6 	.word	0xca62c1d6
