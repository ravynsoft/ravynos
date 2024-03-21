#source: ./ftoi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 97 00                      	ftoi	r0, r0
   3:	fc 97 0f                      	ftoi	r0, r15
   6:	fc 97 f0                      	ftoi	r15, r0
   9:	fc 97 ff                      	ftoi	r15, r15
   c:	fc 94 00                      	ftoi	\[r0\]\.l, r0
   f:	fc 94 0f                      	ftoi	\[r0\]\.l, r15
  12:	fc 94 f0                      	ftoi	\[r15\]\.l, r0
  15:	fc 94 ff                      	ftoi	\[r15\]\.l, r15
  18:	fc 95 00 3f                   	ftoi	252\[r0\]\.l, r0
  1c:	fc 95 0f 3f                   	ftoi	252\[r0\]\.l, r15
  20:	fc 95 f0 3f                   	ftoi	252\[r15\]\.l, r0
  24:	fc 95 ff 3f                   	ftoi	252\[r15\]\.l, r15
  28:	fc 96 00 ff 3f                	ftoi	65532\[r0\]\.l, r0
  2d:	fc 96 0f ff 3f                	ftoi	65532\[r0\]\.l, r15
  32:	fc 96 f0 ff 3f                	ftoi	65532\[r15\]\.l, r0
  37:	fc 96 ff ff 3f                	ftoi	65532\[r15\]\.l, r15
