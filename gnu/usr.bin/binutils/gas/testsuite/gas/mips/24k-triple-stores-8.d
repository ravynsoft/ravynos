#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Range Check >= 24)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a1130000 	sb	s3,0\(t0\)
   4:	a1130001 	sb	s3,1\(t0\)
   8:	00000000 	nop
   c:	a1130018 	sb	s3,24\(t0\)
  10:	0000000d 	break
  14:	a1130000 	sb	s3,0\(t0\)
  18:	a1130001 	sb	s3,1\(t0\)
  1c:	a1130019 	sb	s3,25\(t0\)
  20:	0000000d 	break
  24:	a1130001 	sb	s3,1\(t0\)
  28:	a1130019 	sb	s3,25\(t0\)
  2c:	a113001a 	sb	s3,26\(t0\)
  30:	0000000d 	break
  34:	a1130000 	sb	s3,0\(t0\)
  38:	a5130003 	sh	s3,3\(t0\)
  3c:	00000000 	nop
  40:	a113001a 	sb	s3,26\(t0\)
  44:	0000000d 	break
  48:	a5130000 	sh	s3,0\(t0\)
  4c:	a1130003 	sb	s3,3\(t0\)
  50:	a113001a 	sb	s3,26\(t0\)
  54:	0000000d 	break
  58:	a1130023 	sb	s3,35\(t0\)
  5c:	a5130020 	sh	s3,32\(t0\)
  60:	a1130009 	sb	s3,9\(t0\)
  64:	0000000d 	break
  68:	a1130001 	sb	s3,1\(t0\)
  6c:	a5130019 	sh	s3,25\(t0\)
  70:	a113001b 	sb	s3,27\(t0\)
  74:	0000000d 	break
  78:	a1130000 	sb	s3,0\(t0\)
  7c:	ad130007 	sw	s3,7\(t0\)
  80:	00000000 	nop
  84:	a113001c 	sb	s3,28\(t0\)
  88:	0000000d 	break
  8c:	a1130000 	sb	s3,0\(t0\)
  90:	a1130007 	sb	s3,7\(t0\)
  94:	ad13001c 	sw	s3,28\(t0\)
  98:	0000000d 	break
  9c:	a1130040 	sb	s3,64\(t0\)
  a0:	ad13003b 	sw	s3,59\(t0\)
  a4:	00000000 	nop
  a8:	ad130025 	sw	s3,37\(t0\)
  ac:	0000000d 	break
  b0:	ad130040 	sw	s3,64\(t0\)
  b4:	a113003d 	sb	s3,61\(t0\)
  b8:	a1130027 	sb	s3,39\(t0\)
  bc:	0000000d 	break
  c0:	a1130001 	sb	s3,1\(t0\)
  c4:	ad130019 	sw	s3,25\(t0\)
  c8:	a113001d 	sb	s3,29\(t0\)
  cc:	0000000d 	break
	\.\.\.
