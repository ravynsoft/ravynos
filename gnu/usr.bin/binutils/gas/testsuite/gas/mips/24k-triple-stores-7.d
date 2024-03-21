#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Extended Range Check)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a1130004 	sb	s3,4\(t0\)
   4:	ad130008 	sw	s3,8\(t0\)
   8:	a113000f 	sb	s3,15\(t0\)
   c:	0000000d 	break
  10:	a1130003 	sb	s3,3\(t0\)
  14:	ad130008 	sw	s3,8\(t0\)
  18:	00000000 	nop
  1c:	a113000f 	sb	s3,15\(t0\)
  20:	0000000d 	break
  24:	ad13001c 	sw	s3,28\(t0\)
  28:	ad130008 	sw	s3,8\(t0\)
  2c:	a113001f 	sb	s3,31\(t0\)
  30:	0000000d 	break
  34:	a1130005 	sb	s3,5\(t0\)
  38:	ad130009 	sw	s3,9\(t0\)
  3c:	a1130010 	sb	s3,16\(t0\)
  40:	0000000d 	break
  44:	a1130004 	sb	s3,4\(t0\)
  48:	ad130009 	sw	s3,9\(t0\)
  4c:	00000000 	nop
  50:	a1130010 	sb	s3,16\(t0\)
  54:	0000000d 	break
  58:	a1130006 	sb	s3,6\(t0\)
  5c:	a5130008 	sh	s3,8\(t0\)
  60:	a113000f 	sb	s3,15\(t0\)
  64:	0000000d 	break
  68:	a1130005 	sb	s3,5\(t0\)
  6c:	a5130008 	sh	s3,8\(t0\)
  70:	00000000 	nop
  74:	a113000f 	sb	s3,15\(t0\)
  78:	0000000d 	break
  7c:	a513001e 	sh	s3,30\(t0\)
  80:	a5130008 	sh	s3,8\(t0\)
  84:	a113001f 	sb	s3,31\(t0\)
  88:	0000000d 	break
  8c:	a1130007 	sb	s3,7\(t0\)
  90:	a5130009 	sh	s3,9\(t0\)
  94:	a1130010 	sb	s3,16\(t0\)
  98:	0000000d 	break
  9c:	a1130006 	sb	s3,6\(t0\)
  a0:	a5130009 	sh	s3,9\(t0\)
  a4:	00000000 	nop
  a8:	a1130010 	sb	s3,16\(t0\)
  ac:	0000000d 	break
  b0:	a1130007 	sb	s3,7\(t0\)
  b4:	f5000008 	sdc1	\$f0,8\(t0\)
  b8:	a113000f 	sb	s3,15\(t0\)
  bc:	0000000d 	break
  c0:	a1130007 	sb	s3,7\(t0\)
  c4:	f5000008 	sdc1	\$f0,8\(t0\)
  c8:	00000000 	nop
  cc:	a1130010 	sb	s3,16\(t0\)
  d0:	0000000d 	break
  d4:	a1130010 	sb	s3,16\(t0\)
  d8:	f5000008 	sdc1	\$f0,8\(t0\)
  dc:	a1130017 	sb	s3,23\(t0\)
  e0:	0000000d 	break
  e4:	a1130010 	sb	s3,16\(t0\)
  e8:	f5000008 	sdc1	\$f0,8\(t0\)
  ec:	00000000 	nop
  f0:	a1130018 	sb	s3,24\(t0\)
  f4:	0000000d 	break
  f8:	a1130008 	sb	s3,8\(t0\)
  fc:	f5000009 	sdc1	\$f0,9\(t0\)
 100:	a1130010 	sb	s3,16\(t0\)
 104:	0000000d 	break
 108:	a113fffd 	sb	s3,-3\(t0\)
 10c:	f500fffe 	sdc1	\$f0,-2\(t0\)
 110:	00000000 	nop
 114:	a1130006 	sb	s3,6\(t0\)
 118:	0000000d 	break
	...
