#as: -a32
#source: xcoff-branch-1.s
#objdump: -dr
#name: XCOFF branch test 1 (32-bit)

.*


Disassembly of section \.text:

0+00 <\.foo>:
   0:	48 00 00 29 	bl      28 <foo2\+0x4>
   4:	48 00 00 1d 	bl      20 <foo1\+0xc>
   8:	48 00 00 0d 	bl      14 <foo1>
   c:	48 00 00 19 	bl      24 <foo2>
  10:	48 00 00 1d 	bl      2c <\.bar>
			10: R_(RBR_26|BR)	.*

0+14 <foo1>:
  14:	4b ff ff ed 	bl      0 <\.foo>
  18:	48 00 00 3d 	bl      54 <\.frob>
			18: R_(RBR_26|BR)	.*
  1c:	4b ff ff f5 	bl      10 <\.foo\+0x10>
  20:	48 00 00 15 	bl      34 <\.bar\+0x8>
			20: R_(RBR_26|BR)	.*

0+24 <foo2>:
  24:	48 00 00 41 	bl      64 <\.frob\+0x10>
			24: R_(RBR_26|BR)	.*
  28:	4e 80 00 20 	br

0+2c <\.bar>:
  2c:	4b ff ff e9 	bl      14 <foo1>
			2c: R_(RBR_26|BR)	.*
  30:	4b ff ff f5 	bl      24 <foo2>
			30: R_(RBR_26|BR)	.*
  34:	4b ff ff e9 	bl      1c <foo1\+0x8>
			34: R_(RBR_26|BR)	.*
  38:	4b ff ff f1 	bl      28 <foo2\+0x4>
			38: R_(RBR_26|BR)	.*
  3c:	4b ff ff c5 	bl      0 <\.foo>
			3c: R_(RBR_26|BR)	.*
  40:	4b ff ff ed 	bl      2c <\.bar>
  44:	48 00 00 11 	bl      54 <\.frob>
			44: R_(RBR_26|BR)	.*
  48:	4b ff ff d5 	bl      1c <foo1\+0x8>
			48: R_(RBR_26|BR)	.*
  4c:	4b ff ff ed 	bl      38 <\.bar\+0xc>
  50:	48 00 00 09 	bl      58 <\.frob\+0x4>
			50: R_(RBR_26|BR)	.*

0+54 <\.frob>:
  54:	4b ff ff ad 	bl      0 <\.foo>
			54: R_(RBR_26|BR)	.*
  58:	4b ff ff d5 	bl      2c <\.bar>
			58: R_(RBR_26|BR)	.*
  5c:	4b ff ff f9 	bl      54 <\.frob>
  60:	4b ff ff b5 	bl      14 <foo1>
			60: R_(RBR_26|BR)	.*
  64:	4b ff ff c1 	bl      24 <foo2>
			64: R_(RBR_26|BR)	.*
