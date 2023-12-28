#source: ./bclr.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	f0 08                         	bclr	#0, \[r0\]\.b
   2:	f0 f8                         	bclr	#0, \[r15\]\.b
   4:	f1 08 fc                      	bclr	#0, 252\[r0\]\.b
   7:	f1 f8 fc                      	bclr	#0, 252\[r15\]\.b
   a:	f2 08 fc ff                   	bclr	#0, 65532\[r0\]\.b
   e:	f2 f8 fc ff                   	bclr	#0, 65532\[r15\]\.b
  12:	f0 0f                         	bclr	#7, \[r0\]\.b
  14:	f0 ff                         	bclr	#7, \[r15\]\.b
  16:	f1 0f fc                      	bclr	#7, 252\[r0\]\.b
  19:	f1 ff fc                      	bclr	#7, 252\[r15\]\.b
  1c:	f2 0f fc ff                   	bclr	#7, 65532\[r0\]\.b
  20:	f2 ff fc ff                   	bclr	#7, 65532\[r15\]\.b
  24:	fc 64 00                      	bclr	r0, \[r0\]\.b
  27:	fc 64 f0                      	bclr	r0, \[r15\]\.b
  2a:	fc 65 00 fc                   	bclr	r0, 252\[r0\]\.b
  2e:	fc 65 f0 fc                   	bclr	r0, 252\[r15\]\.b
  32:	fc 66 00 fc ff                	bclr	r0, 65532\[r0\]\.b
  37:	fc 66 f0 fc ff                	bclr	r0, 65532\[r15\]\.b
  3c:	fc 64 0f                      	bclr	r15, \[r0\]\.b
  3f:	fc 64 ff                      	bclr	r15, \[r15\]\.b
  42:	fc 65 0f fc                   	bclr	r15, 252\[r0\]\.b
  46:	fc 65 ff fc                   	bclr	r15, 252\[r15\]\.b
  4a:	fc 66 0f fc ff                	bclr	r15, 65532\[r0\]\.b
  4f:	fc 66 ff fc ff                	bclr	r15, 65532\[r15\]\.b
  54:	7a 00                         	bclr	#0, r0
  56:	7a 0f                         	bclr	#0, r15
  58:	7b f0                         	bclr	#31, r0
  5a:	7b ff                         	bclr	#31, r15
  5c:	fc 67 00                      	bclr	r0, r0
  5f:	fc 67 f0                      	bclr	r0, r15
  62:	fc 67 0f                      	bclr	r15, r0
  65:	fc 67 ff                      	bclr	r15, r15
