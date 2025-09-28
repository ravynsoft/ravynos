#objdump: -dr
#as: -march=armv8-a+crypto

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	4e284be7 	aese	v7.16b, v31.16b
   4:	4e285be7 	aesd	v7.16b, v31.16b
   8:	4e286be7 	aesmc	v7.16b, v31.16b
   c:	4e287be7 	aesimc	v7.16b, v31.16b
  10:	5e280be7 	sha1h	s7, s31
  14:	5e281be7 	sha1su1	v7.4s, v31.4s
  18:	5e282be7 	sha256su0	v7.4s, v31.4s
  1c:	5e1f01e7 	sha1c	q7, s15, v31.4s
  20:	5e1f11e7 	sha1p	q7, s15, v31.4s
  24:	5e1f21e7 	sha1m	q7, s15, v31.4s
  28:	5e1f31e7 	sha1su0	v7.4s, v15.4s, v31.4s
  2c:	5e1f41e7 	sha256h	q7, q15, v31.4s
  30:	5e1f51e7 	sha256h2	q7, q15, v31.4s
  34:	5e1f61e7 	sha256su1	v7.4s, v15.4s, v31.4s
  38:	0e3fe1e7 	pmull	v7.8h, v15.8b, v31.8b
  3c:	0effe1e7 	pmull	v7.1q, v15.1d, v31.1d
  40:	4e3fe1e7 	pmull2	v7.8h, v15.16b, v31.16b
  44:	4effe1e7 	pmull2	v7.1q, v15.2d, v31.2d
