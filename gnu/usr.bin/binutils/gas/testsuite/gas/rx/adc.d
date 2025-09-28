#source: ./adc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 20 80                   	adc	#-128, r0
   4:	fd 74 2f 80                   	adc	#-128, r15
   8:	fd 74 20 7f                   	adc	#127, r0
   c:	fd 74 2f 7f                   	adc	#127, r15
  10:	fd 78 20 00 80                	adc	#0xffff8000, r0
  15:	fd 78 2f 00 80                	adc	#0xffff8000, r15
  1a:	fd 7c 20 00 80 00             	adc	#0x8000, r0
  20:	fd 7c 2f 00 80 00             	adc	#0x8000, r15
  26:	fd 7c 20 00 00 80             	adc	#0xff800000, r0
  2c:	fd 7c 2f 00 00 80             	adc	#0xff800000, r15
  32:	fd 7c 20 ff ff 7f             	adc	#0x7fffff, r0
  38:	fd 7c 2f ff ff 7f             	adc	#0x7fffff, r15
  3e:	fd 70 20 00 00 00 80          	adc	#0x80000000, r0
  45:	fd 70 2f 00 00 00 80          	adc	#0x80000000, r15
  4c:	fd 70 20 ff ff ff 7f          	adc	#0x7fffffff, r0
  53:	fd 70 2f ff ff ff 7f          	adc	#0x7fffffff, r15
  5a:	fc 0b 00                      	adc	r0, r0
  5d:	fc 0b 0f                      	adc	r0, r15
  60:	fc 0b f0                      	adc	r15, r0
  63:	fc 0b ff                      	adc	r15, r15
  66:	06 a0 02 00                   	adc	\[r0\]\.l, r0
  6a:	06 a0 02 0f                   	adc	\[r0\]\.l, r15
  6e:	06 a0 02 f0                   	adc	\[r15\]\.l, r0
  72:	06 a0 02 ff                   	adc	\[r15\]\.l, r15
  76:	06 a1 02 00 3f                	adc	252\[r0\]\.l, r0
  7b:	06 a1 02 0f 3f                	adc	252\[r0\]\.l, r15
  80:	06 a1 02 f0 3f                	adc	252\[r15\]\.l, r0
  85:	06 a1 02 ff 3f                	adc	252\[r15\]\.l, r15
  8a:	06 a2 02 00 ff 3f             	adc	65532\[r0\]\.l, r0
  90:	06 a2 02 0f ff 3f             	adc	65532\[r0\]\.l, r15
  96:	06 a2 02 f0 ff 3f             	adc	65532\[r15\]\.l, r0
  9c:	06 a2 02 ff ff 3f             	adc	65532\[r15\]\.l, r15
