#source: ./btst.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	f4 00                         	btst	#0, \[r0\]\.b
   2:	f4 f0                         	btst	#0, \[r15\]\.b
   4:	f5 00 fc                      	btst	#0, 252\[r0\]\.b
   7:	f5 f0 fc                      	btst	#0, 252\[r15\]\.b
   a:	f6 00 fc ff                   	btst	#0, 65532\[r0\]\.b
   e:	f6 f0 fc ff                   	btst	#0, 65532\[r15\]\.b
  12:	f4 07                         	btst	#7, \[r0\]\.b
  14:	f4 f7                         	btst	#7, \[r15\]\.b
  16:	f5 07 fc                      	btst	#7, 252\[r0\]\.b
  19:	f5 f7 fc                      	btst	#7, 252\[r15\]\.b
  1c:	f6 07 fc ff                   	btst	#7, 65532\[r0\]\.b
  20:	f6 f7 fc ff                   	btst	#7, 65532\[r15\]\.b
  24:	fc 68 00                      	btst	r0, \[r0\]\.b
  27:	fc 68 f0                      	btst	r0, \[r15\]\.b
  2a:	fc 69 00 fc                   	btst	r0, 252\[r0\]\.b
  2e:	fc 69 f0 fc                   	btst	r0, 252\[r15\]\.b
  32:	fc 6a 00 fc ff                	btst	r0, 65532\[r0\]\.b
  37:	fc 6a f0 fc ff                	btst	r0, 65532\[r15\]\.b
  3c:	fc 68 0f                      	btst	r15, \[r0\]\.b
  3f:	fc 68 ff                      	btst	r15, \[r15\]\.b
  42:	fc 69 0f fc                   	btst	r15, 252\[r0\]\.b
  46:	fc 69 ff fc                   	btst	r15, 252\[r15\]\.b
  4a:	fc 6a 0f fc ff                	btst	r15, 65532\[r0\]\.b
  4f:	fc 6a ff fc ff                	btst	r15, 65532\[r15\]\.b
  54:	7c 00                         	btst	#0, r0
  56:	7c 0f                         	btst	#0, r15
  58:	7d f0                         	btst	#31, r0
  5a:	7d ff                         	btst	#31, r15
  5c:	fc 6b 00                      	btst	r0, r0
  5f:	fc 6b f0                      	btst	r0, r15
  62:	fc 6b 0f                      	btst	r15, r0
  65:	fc 6b ff                      	btst	r15, r15
