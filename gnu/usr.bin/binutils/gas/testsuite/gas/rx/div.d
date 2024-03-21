#source: ./div.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 80 80                   	div	#-128, r0
   4:	fd 74 8f 80                   	div	#-128, r15
   8:	fd 74 80 7f                   	div	#127, r0
   c:	fd 74 8f 7f                   	div	#127, r15
  10:	fd 78 80 00 80                	div	#0xffff8000, r0
  15:	fd 78 8f 00 80                	div	#0xffff8000, r15
  1a:	fd 7c 80 00 80 00             	div	#0x8000, r0
  20:	fd 7c 8f 00 80 00             	div	#0x8000, r15
  26:	fd 7c 80 00 00 80             	div	#0xff800000, r0
  2c:	fd 7c 8f 00 00 80             	div	#0xff800000, r15
  32:	fd 7c 80 ff ff 7f             	div	#0x7fffff, r0
  38:	fd 7c 8f ff ff 7f             	div	#0x7fffff, r15
  3e:	fd 70 80 00 00 00 80          	div	#0x80000000, r0
  45:	fd 70 8f 00 00 00 80          	div	#0x80000000, r15
  4c:	fd 70 80 ff ff ff 7f          	div	#0x7fffffff, r0
  53:	fd 70 8f ff ff ff 7f          	div	#0x7fffffff, r15
  5a:	fc 23 00                      	div	r0, r0
  5d:	fc 23 0f                      	div	r0, r15
  60:	fc 23 f0                      	div	r15, r0
  63:	fc 23 ff                      	div	r15, r15
  66:	fc 20 00                      	div	\[r0\]\.ub, r0
  69:	fc 20 0f                      	div	\[r0\]\.ub, r15
  6c:	06 20 08 00                   	div	\[r0\]\.b, r0
  70:	06 20 08 0f                   	div	\[r0\]\.b, r15
  74:	06 e0 08 00                   	div	\[r0\]\.uw, r0
  78:	06 e0 08 0f                   	div	\[r0\]\.uw, r15
  7c:	06 60 08 00                   	div	\[r0\]\.w, r0
  80:	06 60 08 0f                   	div	\[r0\]\.w, r15
  84:	06 a0 08 00                   	div	\[r0\]\.l, r0
  88:	06 a0 08 0f                   	div	\[r0\]\.l, r15
  8c:	fc 20 f0                      	div	\[r15\]\.ub, r0
  8f:	fc 20 ff                      	div	\[r15\]\.ub, r15
  92:	06 20 08 f0                   	div	\[r15\]\.b, r0
  96:	06 20 08 ff                   	div	\[r15\]\.b, r15
  9a:	06 e0 08 f0                   	div	\[r15\]\.uw, r0
  9e:	06 e0 08 ff                   	div	\[r15\]\.uw, r15
  a2:	06 60 08 f0                   	div	\[r15\]\.w, r0
  a6:	06 60 08 ff                   	div	\[r15\]\.w, r15
  aa:	06 a0 08 f0                   	div	\[r15\]\.l, r0
  ae:	06 a0 08 ff                   	div	\[r15\]\.l, r15
  b2:	fc 21 00 fc                   	div	252\[r0\]\.ub, r0
  b6:	fc 21 0f fc                   	div	252\[r0\]\.ub, r15
  ba:	06 21 08 00 fc                	div	252\[r0\]\.b, r0
  bf:	06 21 08 0f fc                	div	252\[r0\]\.b, r15
  c4:	06 e1 08 00 7e                	div	252\[r0\]\.uw, r0
  c9:	06 e1 08 0f 7e                	div	252\[r0\]\.uw, r15
  ce:	06 61 08 00 7e                	div	252\[r0\]\.w, r0
  d3:	06 61 08 0f 7e                	div	252\[r0\]\.w, r15
  d8:	06 a1 08 00 3f                	div	252\[r0\]\.l, r0
  dd:	06 a1 08 0f 3f                	div	252\[r0\]\.l, r15
  e2:	fc 21 f0 fc                   	div	252\[r15\]\.ub, r0
  e6:	fc 21 ff fc                   	div	252\[r15\]\.ub, r15
  ea:	06 21 08 f0 fc                	div	252\[r15\]\.b, r0
  ef:	06 21 08 ff fc                	div	252\[r15\]\.b, r15
  f4:	06 e1 08 f0 7e                	div	252\[r15\]\.uw, r0
  f9:	06 e1 08 ff 7e                	div	252\[r15\]\.uw, r15
  fe:	06 61 08 f0 7e                	div	252\[r15\]\.w, r0
 103:	06 61 08 ff 7e                	div	252\[r15\]\.w, r15
 108:	06 a1 08 f0 3f                	div	252\[r15\]\.l, r0
 10d:	06 a1 08 ff 3f                	div	252\[r15\]\.l, r15
 112:	fc 22 00 fc ff                	div	65532\[r0\]\.ub, r0
 117:	fc 22 0f fc ff                	div	65532\[r0\]\.ub, r15
 11c:	06 22 08 00 fc ff             	div	65532\[r0\]\.b, r0
 122:	06 22 08 0f fc ff             	div	65532\[r0\]\.b, r15
 128:	06 e2 08 00 fe 7f             	div	65532\[r0\]\.uw, r0
 12e:	06 e2 08 0f fe 7f             	div	65532\[r0\]\.uw, r15
 134:	06 62 08 00 fe 7f             	div	65532\[r0\]\.w, r0
 13a:	06 62 08 0f fe 7f             	div	65532\[r0\]\.w, r15
 140:	06 a2 08 00 ff 3f             	div	65532\[r0\]\.l, r0
 146:	06 a2 08 0f ff 3f             	div	65532\[r0\]\.l, r15
 14c:	fc 22 f0 fc ff                	div	65532\[r15\]\.ub, r0
 151:	fc 22 ff fc ff                	div	65532\[r15\]\.ub, r15
 156:	06 22 08 f0 fc ff             	div	65532\[r15\]\.b, r0
 15c:	06 22 08 ff fc ff             	div	65532\[r15\]\.b, r15
 162:	06 e2 08 f0 fe 7f             	div	65532\[r15\]\.uw, r0
 168:	06 e2 08 ff fe 7f             	div	65532\[r15\]\.uw, r15
 16e:	06 62 08 f0 fe 7f             	div	65532\[r15\]\.w, r0
 174:	06 62 08 ff fe 7f             	div	65532\[r15\]\.w, r15
 17a:	06 a2 08 f0 ff 3f             	div	65532\[r15\]\.l, r0
 180:	06 a2 08 ff ff 3f             	div	65532\[r15\]\.l, r15
