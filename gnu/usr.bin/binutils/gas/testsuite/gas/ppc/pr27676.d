#as: -mpower10
#objdump: -dr -Mpower10

.*:     file format .*

Disassembly of section \.text:

0+ <\.text>:
   0:	(7c 03 22 2c|2c 22 03 7c) 	dcbtct  r3,r4
   4:	(7c 23 22 2c|2c 22 23 7c) 	dcbtct  r3,r4,1
   8:	(7c 43 22 2c|2c 22 43 7c) 	dcbtct  r3,r4,2
   c:	(7c 63 22 2c|2c 22 63 7c) 	dcbtct  r3,r4,3
  10:	(7c 83 22 2c|2c 22 83 7c) 	dcbtct  r3,r4,4
  14:	(7c a3 22 2c|2c 22 a3 7c) 	dcbtct  r3,r4,5
  18:	(7c c3 22 2c|2c 22 c3 7c) 	dcbtct  r3,r4,6
  1c:	(7c e3 22 2c|2c 22 e3 7c) 	dcbtct  r3,r4,7
  20:	(7d 03 22 2c|2c 22 03 7d) 	dcbtds  r3,r4
  24:	(7d 23 22 2c|2c 22 23 7d) 	dcbtds  r3,r4,9
  28:	(7d 43 22 2c|2c 22 43 7d) 	dcbtds  r3,r4,10
  2c:	(7d 63 22 2c|2c 22 63 7d) 	dcbtds  r3,r4,11
  30:	(7d 83 22 2c|2c 22 83 7d) 	dcbtds  r3,r4,12
  34:	(7d a3 22 2c|2c 22 a3 7d) 	dcbtds  r3,r4,13
  38:	(7d c3 22 2c|2c 22 c3 7d) 	dcbtds  r3,r4,14
  3c:	(7d e3 22 2c|2c 22 e3 7d) 	dcbtds  r3,r4,15
  40:	(7e 03 22 2c|2c 22 03 7e) 	dcbtt   r3,r4
  44:	(7e 23 22 2c|2c 22 23 7e) 	dcbna   r3,r4
  48:	(7e 43 22 2c|2c 22 43 7e) 	dcbt    r3,r4,18
  4c:	(7e 63 22 2c|2c 22 63 7e) 	dcbt    r3,r4,19
  50:	(7e 83 22 2c|2c 22 83 7e) 	dcbt    r3,r4,20
  54:	(7e a3 22 2c|2c 22 a3 7e) 	dcbt    r3,r4,21
  58:	(7e c3 22 2c|2c 22 c3 7e) 	dcbt    r3,r4,22
  5c:	(7e e3 22 2c|2c 22 e3 7e) 	dcbt    r3,r4,23
  60:	(7f 03 22 2c|2c 22 03 7f) 	dcbt    r3,r4,24
  64:	(7f 23 22 2c|2c 22 23 7f) 	dcbt    r3,r4,25
  68:	(7f 43 22 2c|2c 22 43 7f) 	dcbt    r3,r4,26
  6c:	(7f 63 22 2c|2c 22 63 7f) 	dcbt    r3,r4,27
  70:	(7f 83 22 2c|2c 22 83 7f) 	dcbt    r3,r4,28
  74:	(7f a3 22 2c|2c 22 a3 7f) 	dcbt    r3,r4,29
  78:	(7f c3 22 2c|2c 22 c3 7f) 	dcbt    r3,r4,30
  7c:	(7f e3 22 2c|2c 22 e3 7f) 	dcbt    r3,r4,31
  80:	(7c 05 32 2c|2c 32 05 7c) 	dcbtct  r5,r6
  84:	(7d 05 32 2c|2c 32 05 7d) 	dcbtds  r5,r6
  88:	(7e 05 32 2c|2c 32 05 7e) 	dcbtt   r5,r6
  8c:	(7e 25 32 2c|2c 32 25 7e) 	dcbna   r5,r6
  90:	(7c 03 21 ec|ec 21 03 7c) 	dcbtstct r3,r4
  94:	(7c 23 21 ec|ec 21 23 7c) 	dcbtstct r3,r4,1
  98:	(7c 43 21 ec|ec 21 43 7c) 	dcbtstct r3,r4,2
  9c:	(7c 63 21 ec|ec 21 63 7c) 	dcbtstct r3,r4,3
  a0:	(7c 83 21 ec|ec 21 83 7c) 	dcbtstct r3,r4,4
  a4:	(7c a3 21 ec|ec 21 a3 7c) 	dcbtstct r3,r4,5
  a8:	(7c c3 21 ec|ec 21 c3 7c) 	dcbtstct r3,r4,6
  ac:	(7c e3 21 ec|ec 21 e3 7c) 	dcbtstct r3,r4,7
  b0:	(7d 03 21 ec|ec 21 03 7d) 	dcbtstds r3,r4
  b4:	(7d 23 21 ec|ec 21 23 7d) 	dcbtstds r3,r4,9
  b8:	(7d 43 21 ec|ec 21 43 7d) 	dcbtstds r3,r4,10
  bc:	(7d 63 21 ec|ec 21 63 7d) 	dcbtstds r3,r4,11
  c0:	(7d 83 21 ec|ec 21 83 7d) 	dcbtstds r3,r4,12
  c4:	(7d a3 21 ec|ec 21 a3 7d) 	dcbtstds r3,r4,13
  c8:	(7d c3 21 ec|ec 21 c3 7d) 	dcbtstds r3,r4,14
  cc:	(7d e3 21 ec|ec 21 e3 7d) 	dcbtstds r3,r4,15
  d0:	(7e 03 21 ec|ec 21 03 7e) 	dcbtstt r3,r4
  d4:	(7e 23 21 ec|ec 21 23 7e) 	dcbtst  r3,r4,17
  d8:	(7e 43 21 ec|ec 21 43 7e) 	dcbtst  r3,r4,18
  dc:	(7e 63 21 ec|ec 21 63 7e) 	dcbtst  r3,r4,19
  e0:	(7e 83 21 ec|ec 21 83 7e) 	dcbtst  r3,r4,20
  e4:	(7e a3 21 ec|ec 21 a3 7e) 	dcbtst  r3,r4,21
  e8:	(7e c3 21 ec|ec 21 c3 7e) 	dcbtst  r3,r4,22
  ec:	(7e e3 21 ec|ec 21 e3 7e) 	dcbtst  r3,r4,23
  f0:	(7f 03 21 ec|ec 21 03 7f) 	dcbtst  r3,r4,24
  f4:	(7f 23 21 ec|ec 21 23 7f) 	dcbtst  r3,r4,25
  f8:	(7f 43 21 ec|ec 21 43 7f) 	dcbtst  r3,r4,26
  fc:	(7f 63 21 ec|ec 21 63 7f) 	dcbtst  r3,r4,27
 100:	(7f 83 21 ec|ec 21 83 7f) 	dcbtst  r3,r4,28
 104:	(7f a3 21 ec|ec 21 a3 7f) 	dcbtst  r3,r4,29
 108:	(7f c3 21 ec|ec 21 c3 7f) 	dcbtst  r3,r4,30
 10c:	(7f e3 21 ec|ec 21 e3 7f) 	dcbtst  r3,r4,31
 110:	(7c 05 31 ec|ec 31 05 7c) 	dcbtstct r5,r6
 114:	(7d 05 31 ec|ec 31 05 7d) 	dcbtstds r5,r6
 118:	(7e 05 31 ec|ec 31 05 7e) 	dcbtstt r5,r6
