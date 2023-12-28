#source: ./ftou.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc a7 00                      	ftou	r0, r0
   3:	fc a7 0f                      	ftou	r0, r15
   6:	fc a7 f0                      	ftou	r15, r0
   9:	fc a7 ff                      	ftou	r15, r15
   c:	fc a4 00                      	ftou	\[r0\]\.l, r0
   f:	fc a4 0f                      	ftou	\[r0\]\.l, r15
  12:	fc a4 f0                      	ftou	\[r15\]\.l, r0
  15:	fc a4 ff                      	ftou	\[r15\]\.l, r15
  18:	fc a5 00 3f                   	ftou	252\[r0\]\.l, r0
  1c:	fc a5 0f 3f                   	ftou	252\[r0\]\.l, r15
  20:	fc a5 f0 3f                   	ftou	252\[r15\]\.l, r0
  24:	fc a5 ff 3f                   	ftou	252\[r15\]\.l, r15
  28:	fc a6 00 ff 3f                	ftou	65532\[r0\]\.l, r0
  2d:	fc a6 0f ff 3f                	ftou	65532\[r0\]\.l, r15
  32:	fc a6 f0 ff 3f                	ftou	65532\[r15\]\.l, r0
  37:	fc a6 ff ff 3f                	ftou	65532\[r15\]\.l, r15
