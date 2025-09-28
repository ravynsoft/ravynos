#source: ./bnot.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc e0 0f                      	bnot	#0, \[r0\]\.b
   3:	fc e0 ff                      	bnot	#0, \[r15\]\.b
   6:	fc e1 0f fc                   	bnot	#0, 252\[r0\]\.b
   a:	fc e1 ff fc                   	bnot	#0, 252\[r15\]\.b
   e:	fc e2 0f fc ff                	bnot	#0, 65532\[r0\]\.b
  13:	fc e2 ff fc ff                	bnot	#0, 65532\[r15\]\.b
  18:	fc fc 0f                      	bnot	#7, \[r0\]\.b
  1b:	fc fc ff                      	bnot	#7, \[r15\]\.b
  1e:	fc fd 0f fc                   	bnot	#7, 252\[r0\]\.b
  22:	fc fd ff fc                   	bnot	#7, 252\[r15\]\.b
  26:	fc fe 0f fc ff                	bnot	#7, 65532\[r0\]\.b
  2b:	fc fe ff fc ff                	bnot	#7, 65532\[r15\]\.b
  30:	fc 6c 00                      	bnot	r0, \[r0\]\.b
  33:	fc 6c f0                      	bnot	r0, \[r15\]\.b
  36:	fc 6d 00 fc                   	bnot	r0, 252\[r0\]\.b
  3a:	fc 6d f0 fc                   	bnot	r0, 252\[r15\]\.b
  3e:	fc 6e 00 fc ff                	bnot	r0, 65532\[r0\]\.b
  43:	fc 6e f0 fc ff                	bnot	r0, 65532\[r15\]\.b
  48:	fc 6c 0f                      	bnot	r15, \[r0\]\.b
  4b:	fc 6c ff                      	bnot	r15, \[r15\]\.b
  4e:	fc 6d 0f fc                   	bnot	r15, 252\[r0\]\.b
  52:	fc 6d ff fc                   	bnot	r15, 252\[r15\]\.b
  56:	fc 6e 0f fc ff                	bnot	r15, 65532\[r0\]\.b
  5b:	fc 6e ff fc ff                	bnot	r15, 65532\[r15\]\.b
  60:	fd e0 f0                      	bnot	#0, r0
  63:	fd e0 ff                      	bnot	#0, r15
  66:	fd ff f0                      	bnot	#31, r0
  69:	fd ff ff                      	bnot	#31, r15
  6c:	fc 6f 00                      	bnot	r0, r0
  6f:	fc 6f f0                      	bnot	r0, r15
  72:	fc 6f 0f                      	bnot	r15, r0
  75:	fc 6f ff                      	bnot	r15, r15
