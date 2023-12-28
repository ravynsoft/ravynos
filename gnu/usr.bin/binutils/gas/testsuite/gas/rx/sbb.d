#source: ./sbb.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 03 00                      	sbb	r0, r0
   3:	fc 03 0f                      	sbb	r0, r15
   6:	fc 03 f0                      	sbb	r15, r0
   9:	fc 03 ff                      	sbb	r15, r15
   c:	06 a0 00 00                   	sbb	\[r0\]\.l, r0
  10:	06 a0 00 0f                   	sbb	\[r0\]\.l, r15
  14:	06 a0 00 f0                   	sbb	\[r15\]\.l, r0
  18:	06 a0 00 ff                   	sbb	\[r15\]\.l, r15
  1c:	06 a1 00 00 3f                	sbb	252\[r0\]\.l, r0
  21:	06 a1 00 0f 3f                	sbb	252\[r0\]\.l, r15
  26:	06 a1 00 f0 3f                	sbb	252\[r15\]\.l, r0
  2b:	06 a1 00 ff 3f                	sbb	252\[r15\]\.l, r15
  30:	06 a2 00 00 ff 3f             	sbb	65532\[r0\]\.l, r0
  36:	06 a2 00 0f ff 3f             	sbb	65532\[r0\]\.l, r15
  3c:	06 a2 00 f0 ff 3f             	sbb	65532\[r15\]\.l, r0
  42:	06 a2 00 ff ff 3f             	sbb	65532\[r15\]\.l, r15
  48:	fd 74 20 7f                   	adc	#127, r0
  4c:	fd 74 2f 7f                   	adc	#127, r15
  50:	fd 74 20 80                   	adc	#-128, r0
  54:	fd 74 2f 80                   	adc	#-128, r15
  58:	fd 78 20 ff 7f                	adc	#0x7fff, r0
  5d:	fd 78 2f ff 7f                	adc	#0x7fff, r15
  62:	fd 7c 20 ff 7f ff             	adc	#0xffff7fff, r0
  68:	fd 7c 2f ff 7f ff             	adc	#0xffff7fff, r15
  6e:	fd 7c 20 ff ff 7f             	adc	#0x7fffff, r0
  74:	fd 7c 2f ff ff 7f             	adc	#0x7fffff, r15
  7a:	fd 7c 20 00 00 80             	adc	#0xff800000, r0
  80:	fd 7c 2f 00 00 80             	adc	#0xff800000, r15
  86:	fd 70 20 ff ff ff 7f          	adc	#0x7fffffff, r0
  8d:	fd 70 2f ff ff ff 7f          	adc	#0x7fffffff, r15
  94:	fd 70 20 00 00 00 80          	adc	#0x80000000, r0
  9b:	fd 70 2f 00 00 00 80          	adc	#0x80000000, r15
