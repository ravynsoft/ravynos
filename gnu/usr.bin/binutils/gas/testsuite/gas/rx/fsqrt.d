#source: ./fsqrt.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc a3 00                      	fsqrt	r0, r0
   3:	fc a3 0f                      	fsqrt	r0, r15
   6:	fc a3 f0                      	fsqrt	r15, r0
   9:	fc a3 ff                      	fsqrt	r15, r15
   c:	fc a0 00                      	fsqrt	\[r0\]\.l, r0
   f:	fc a0 0f                      	fsqrt	\[r0\]\.l, r15
  12:	fc a0 f0                      	fsqrt	\[r15\]\.l, r0
  15:	fc a0 ff                      	fsqrt	\[r15\]\.l, r15
  18:	fc a1 00 3f                   	fsqrt	252\[r0\]\.l, r0
  1c:	fc a1 0f 3f                   	fsqrt	252\[r0\]\.l, r15
  20:	fc a1 f0 3f                   	fsqrt	252\[r15\]\.l, r0
  24:	fc a1 ff 3f                   	fsqrt	252\[r15\]\.l, r15
  28:	fc a2 00 ff 3f                	fsqrt	65532\[r0\]\.l, r0
  2d:	fc a2 0f ff 3f                	fsqrt	65532\[r0\]\.l, r15
  32:	fc a2 f0 ff 3f                	fsqrt	65532\[r15\]\.l, r0
  37:	fc a2 ff ff 3f                	fsqrt	65532\[r15\]\.l, r15
