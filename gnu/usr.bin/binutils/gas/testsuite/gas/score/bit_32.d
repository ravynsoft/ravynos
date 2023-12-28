#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  bit_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	5000      	bitclr!		r0, 0x0
   2:	501f      	bitclr!		r0, 0x1f
   4:	51e0      	bitclr!		r15, 0x0
   6:	51ff      	bitclr!		r15, 0x1f
   8:	5000      	bitclr!		r0, 0x0
   a:	5000      	bitclr!		r0, 0x0
   c:	5000      	bitclr!		r0, 0x0
   e:	5000      	bitclr!		r0, 0x0
  10:	5000      	bitclr!		r0, 0x0
  12:	5000      	bitclr!		r0, 0x0
  14:	5000      	bitclr!		r0, 0x0
  16:	5000      	bitclr!		r0, 0x0
  18:	8000 0029 	bitclr.c	r0, r0, 0x0
  1c:	8210 0028 	bitclr	r16, r16, 0x0
  20:	8210 7c28 	bitclr	r16, r16, 0x1f
  24:	5200      	bitset!		r0, 0x0
  26:	521f      	bitset!		r0, 0x1f
  28:	53e0      	bitset!		r15, 0x0
  2a:	53ff      	bitset!		r15, 0x1f
  2c:	5200      	bitset!		r0, 0x0
  2e:	5200      	bitset!		r0, 0x0
  30:	5200      	bitset!		r0, 0x0
  32:	5200      	bitset!		r0, 0x0
  34:	5200      	bitset!		r0, 0x0
  36:	5200      	bitset!		r0, 0x0
  38:	5200      	bitset!		r0, 0x0
  3a:	5200      	bitset!		r0, 0x0
  3c:	8000 002b 	bitset.c	r0, r0, 0x0
  40:	8210 002a 	bitset	r16, r16, 0x0
  44:	8210 7c2a 	bitset	r16, r16, 0x1f
  48:	5600      	bittgl!		r0, 0x0
  4a:	561f      	bittgl!		r0, 0x1f
  4c:	57e0      	bittgl!		r15, 0x0
  4e:	57ff      	bittgl!		r15, 0x1f
  50:	5600      	bittgl!		r0, 0x0
  52:	5600      	bittgl!		r0, 0x0
  54:	5600      	bittgl!		r0, 0x0
  56:	5600      	bittgl!		r0, 0x0
  58:	5600      	bittgl!		r0, 0x0
  5a:	5600      	bittgl!		r0, 0x0
  5c:	5600      	bittgl!		r0, 0x0
  5e:	5600      	bittgl!		r0, 0x0
  60:	8000 002f 	bittgl.c	r0, r0, 0x0
  64:	8210 002e 	bittgl	r16, r16, 0x0
  68:	8210 7c2e 	bittgl	r16, r16, 0x1f
  6c:	5400      	bittst!		r0, 0x0
  6e:	541f      	bittst!		r0, 0x1f
  70:	55e0      	bittst!		r15, 0x0
  72:	55ff      	bittst!		r15, 0x1f
  74:	5400      	bittst!		r0, 0x0
  76:	5400      	bittst!		r0, 0x0
  78:	5400      	bittst!		r0, 0x0
  7a:	5400      	bittst!		r0, 0x0
  7c:	5400      	bittst!		r0, 0x0
  7e:	5400      	bittst!		r0, 0x0
  80:	5400      	bittst!		r0, 0x0
  82:	5400      	bittst!		r0, 0x0
  84:	8010 002d 	bittst.c	r16, 0x0
  88:	8010 7c2d 	bittst.c	r16, 0x1f
#pass
