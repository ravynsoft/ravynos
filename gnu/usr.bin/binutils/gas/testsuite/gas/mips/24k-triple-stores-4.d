#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Range Check >= 32)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a113000a 	sb	s3,10\(t0\)
   4:	a5130001 	sh	s3,1\(t0\)
   8:	00000000 	nop
   c:	a1130020 	sb	s3,32\(t0\)
  10:	0000000d 	break
  14:	a113000a 	sb	s3,10\(t0\)
  18:	a1130001 	sb	s3,1\(t0\)
  1c:	a5130020 	sh	s3,32\(t0\)
  20:	0000000d 	break
  24:	a1130021 	sb	s3,33\(t0\)
  28:	a5130037 	sh	s3,55\(t0\)
  2c:	00000000 	nop
  30:	a1130040 	sb	s3,64\(t0\)
  34:	0000000d 	break
  38:	a1130021 	sb	s3,33\(t0\)
  3c:	a1130037 	sb	s3,55\(t0\)
  40:	a5130040 	sh	s3,64\(t0\)
  44:	0000000d 	break
  48:	a113000c 	sb	s3,12\(t0\)
  4c:	ad130001 	sw	s3,1\(t0\)
  50:	00000000 	nop
  54:	a1130020 	sb	s3,32\(t0\)
  58:	0000000d 	break
  5c:	a113000c 	sb	s3,12\(t0\)
  60:	a1130001 	sb	s3,1\(t0\)
  64:	ad130020 	sw	s3,32\(t0\)
  68:	0000000d 	break
  6c:	a1130023 	sb	s3,35\(t0\)
  70:	ad130037 	sw	s3,55\(t0\)
  74:	00000000 	nop
  78:	a1130040 	sb	s3,64\(t0\)
  7c:	0000000d 	break
  80:	a1130023 	sb	s3,35\(t0\)
  84:	a1130037 	sb	s3,55\(t0\)
  88:	ad130040 	sw	s3,64\(t0\)
  8c:	0000000d 	break
  90:	a1130010 	sb	s3,16\(t0\)
  94:	f5000001 	sdc1	\$f0,1\(t0\)
  98:	00000000 	nop
  9c:	a1130020 	sb	s3,32\(t0\)
  a0:	0000000d 	break
  a4:	a1130010 	sb	s3,16\(t0\)
  a8:	a1130001 	sb	s3,1\(t0\)
  ac:	f5000020 	sdc1	\$f0,32\(t0\)
  b0:	0000000d 	break
  b4:	a1130027 	sb	s3,39\(t0\)
  b8:	f5000037 	sdc1	\$f0,55\(t0\)
  bc:	00000000 	nop
  c0:	a1130040 	sb	s3,64\(t0\)
  c4:	0000000d 	break
  c8:	a1130027 	sb	s3,39\(t0\)
  cc:	a1130037 	sb	s3,55\(t0\)
  d0:	f5000040 	sdc1	\$f0,64\(t0\)
  d4:	0000000d 	break
	...
