#source: ./round.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 9b 00                      	round	r0, r0
   3:	fc 9b 0f                      	round	r0, r15
   6:	fc 9b f0                      	round	r15, r0
   9:	fc 9b ff                      	round	r15, r15
   c:	fc 98 00                      	round	\[r0\]\.l, r0
   f:	fc 98 0f                      	round	\[r0\]\.l, r15
  12:	fc 98 f0                      	round	\[r15\]\.l, r0
  15:	fc 98 ff                      	round	\[r15\]\.l, r15
  18:	fc 99 00 3f                   	round	252\[r0\]\.l, r0
  1c:	fc 99 0f 3f                   	round	252\[r0\]\.l, r15
  20:	fc 99 f0 3f                   	round	252\[r15\]\.l, r0
  24:	fc 99 ff 3f                   	round	252\[r15\]\.l, r15
  28:	fc 9a 00 ff 3f                	round	65532\[r0\]\.l, r0
  2d:	fc 9a 0f ff 3f                	round	65532\[r0\]\.l, r15
  32:	fc 9a f0 ff 3f                	round	65532\[r15\]\.l, r0
  37:	fc 9a ff ff 3f                	round	65532\[r15\]\.l, r15
