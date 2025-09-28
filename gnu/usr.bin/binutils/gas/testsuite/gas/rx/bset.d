#source: ./bset.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	f0 00                         	bset	#0, \[r0\]\.b
   2:	f0 f0                         	bset	#0, \[r15\]\.b
   4:	f1 00 fc                      	bset	#0, 252\[r0\]\.b
   7:	f1 f0 fc                      	bset	#0, 252\[r15\]\.b
   a:	f2 00 fc ff                   	bset	#0, 65532\[r0\]\.b
   e:	f2 f0 fc ff                   	bset	#0, 65532\[r15\]\.b
  12:	f0 07                         	bset	#7, \[r0\]\.b
  14:	f0 f7                         	bset	#7, \[r15\]\.b
  16:	f1 07 fc                      	bset	#7, 252\[r0\]\.b
  19:	f1 f7 fc                      	bset	#7, 252\[r15\]\.b
  1c:	f2 07 fc ff                   	bset	#7, 65532\[r0\]\.b
  20:	f2 f7 fc ff                   	bset	#7, 65532\[r15\]\.b
  24:	fc 60 00                      	bset	r0, \[r0\]\.b
  27:	fc 60 f0                      	bset	r0, \[r15\]\.b
  2a:	fc 61 00 fc                   	bset	r0, 252\[r0\]\.b
  2e:	fc 61 f0 fc                   	bset	r0, 252\[r15\]\.b
  32:	fc 62 00 fc ff                	bset	r0, 65532\[r0\]\.b
  37:	fc 62 f0 fc ff                	bset	r0, 65532\[r15\]\.b
  3c:	fc 60 0f                      	bset	r15, \[r0\]\.b
  3f:	fc 60 ff                      	bset	r15, \[r15\]\.b
  42:	fc 61 0f fc                   	bset	r15, 252\[r0\]\.b
  46:	fc 61 ff fc                   	bset	r15, 252\[r15\]\.b
  4a:	fc 62 0f fc ff                	bset	r15, 65532\[r0\]\.b
  4f:	fc 62 ff fc ff                	bset	r15, 65532\[r15\]\.b
  54:	78 00                         	bset	#0, r0
  56:	78 0f                         	bset	#0, r15
  58:	79 f0                         	bset	#31, r0
  5a:	79 ff                         	bset	#31, r15
  5c:	fc 63 00                      	bset	r0, r0
  5f:	fc 63 f0                      	bset	r0, r15
  62:	fc 63 0f                      	bset	r15, r0
  65:	fc 63 ff                      	bset	r15, r15
