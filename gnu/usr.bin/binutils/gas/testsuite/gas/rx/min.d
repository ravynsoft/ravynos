#source: ./min.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 50 80                   	min	#-128, r0
   4:	fd 74 5f 80                   	min	#-128, r15
   8:	fd 74 50 7f                   	min	#127, r0
   c:	fd 74 5f 7f                   	min	#127, r15
  10:	fd 78 50 00 80                	min	#0xffff8000, r0
  15:	fd 78 5f 00 80                	min	#0xffff8000, r15
  1a:	fd 7c 50 00 80 00             	min	#0x8000, r0
  20:	fd 7c 5f 00 80 00             	min	#0x8000, r15
  26:	fd 7c 50 00 00 80             	min	#0xff800000, r0
  2c:	fd 7c 5f 00 00 80             	min	#0xff800000, r15
  32:	fd 7c 50 ff ff 7f             	min	#0x7fffff, r0
  38:	fd 7c 5f ff ff 7f             	min	#0x7fffff, r15
  3e:	fd 70 50 00 00 00 80          	min	#0x80000000, r0
  45:	fd 70 5f 00 00 00 80          	min	#0x80000000, r15
  4c:	fd 70 50 ff ff ff 7f          	min	#0x7fffffff, r0
  53:	fd 70 5f ff ff ff 7f          	min	#0x7fffffff, r15
  5a:	fc 17 00                      	min	r0, r0
  5d:	fc 17 0f                      	min	r0, r15
  60:	fc 17 f0                      	min	r15, r0
  63:	fc 17 ff                      	min	r15, r15
  66:	fc 14 00                      	min	\[r0\]\.ub, r0
  69:	fc 14 0f                      	min	\[r0\]\.ub, r15
  6c:	06 20 05 00                   	min	\[r0\]\.b, r0
  70:	06 20 05 0f                   	min	\[r0\]\.b, r15
  74:	06 e0 05 00                   	min	\[r0\]\.uw, r0
  78:	06 e0 05 0f                   	min	\[r0\]\.uw, r15
  7c:	06 60 05 00                   	min	\[r0\]\.w, r0
  80:	06 60 05 0f                   	min	\[r0\]\.w, r15
  84:	06 a0 05 00                   	min	\[r0\]\.l, r0
  88:	06 a0 05 0f                   	min	\[r0\]\.l, r15
  8c:	fc 14 f0                      	min	\[r15\]\.ub, r0
  8f:	fc 14 ff                      	min	\[r15\]\.ub, r15
  92:	06 20 05 f0                   	min	\[r15\]\.b, r0
  96:	06 20 05 ff                   	min	\[r15\]\.b, r15
  9a:	06 e0 05 f0                   	min	\[r15\]\.uw, r0
  9e:	06 e0 05 ff                   	min	\[r15\]\.uw, r15
  a2:	06 60 05 f0                   	min	\[r15\]\.w, r0
  a6:	06 60 05 ff                   	min	\[r15\]\.w, r15
  aa:	06 a0 05 f0                   	min	\[r15\]\.l, r0
  ae:	06 a0 05 ff                   	min	\[r15\]\.l, r15
  b2:	fc 15 00 fc                   	min	252\[r0\]\.ub, r0
  b6:	fc 15 0f fc                   	min	252\[r0\]\.ub, r15
  ba:	06 21 05 00 fc                	min	252\[r0\]\.b, r0
  bf:	06 21 05 0f fc                	min	252\[r0\]\.b, r15
  c4:	06 e1 05 00 7e                	min	252\[r0\]\.uw, r0
  c9:	06 e1 05 0f 7e                	min	252\[r0\]\.uw, r15
  ce:	06 61 05 00 7e                	min	252\[r0\]\.w, r0
  d3:	06 61 05 0f 7e                	min	252\[r0\]\.w, r15
  d8:	06 a1 05 00 3f                	min	252\[r0\]\.l, r0
  dd:	06 a1 05 0f 3f                	min	252\[r0\]\.l, r15
  e2:	fc 15 f0 fc                   	min	252\[r15\]\.ub, r0
  e6:	fc 15 ff fc                   	min	252\[r15\]\.ub, r15
  ea:	06 21 05 f0 fc                	min	252\[r15\]\.b, r0
  ef:	06 21 05 ff fc                	min	252\[r15\]\.b, r15
  f4:	06 e1 05 f0 7e                	min	252\[r15\]\.uw, r0
  f9:	06 e1 05 ff 7e                	min	252\[r15\]\.uw, r15
  fe:	06 61 05 f0 7e                	min	252\[r15\]\.w, r0
 103:	06 61 05 ff 7e                	min	252\[r15\]\.w, r15
 108:	06 a1 05 f0 3f                	min	252\[r15\]\.l, r0
 10d:	06 a1 05 ff 3f                	min	252\[r15\]\.l, r15
 112:	fc 16 00 fc ff                	min	65532\[r0\]\.ub, r0
 117:	fc 16 0f fc ff                	min	65532\[r0\]\.ub, r15
 11c:	06 22 05 00 fc ff             	min	65532\[r0\]\.b, r0
 122:	06 22 05 0f fc ff             	min	65532\[r0\]\.b, r15
 128:	06 e2 05 00 fe 7f             	min	65532\[r0\]\.uw, r0
 12e:	06 e2 05 0f fe 7f             	min	65532\[r0\]\.uw, r15
 134:	06 62 05 00 fe 7f             	min	65532\[r0\]\.w, r0
 13a:	06 62 05 0f fe 7f             	min	65532\[r0\]\.w, r15
 140:	06 a2 05 00 ff 3f             	min	65532\[r0\]\.l, r0
 146:	06 a2 05 0f ff 3f             	min	65532\[r0\]\.l, r15
 14c:	fc 16 f0 fc ff                	min	65532\[r15\]\.ub, r0
 151:	fc 16 ff fc ff                	min	65532\[r15\]\.ub, r15
 156:	06 22 05 f0 fc ff             	min	65532\[r15\]\.b, r0
 15c:	06 22 05 ff fc ff             	min	65532\[r15\]\.b, r15
 162:	06 e2 05 f0 fe 7f             	min	65532\[r15\]\.uw, r0
 168:	06 e2 05 ff fe 7f             	min	65532\[r15\]\.uw, r15
 16e:	06 62 05 f0 fe 7f             	min	65532\[r15\]\.w, r0
 174:	06 62 05 ff fe 7f             	min	65532\[r15\]\.w, r15
 17a:	06 a2 05 f0 ff 3f             	min	65532\[r15\]\.l, r0
 180:	06 a2 05 ff ff 3f             	min	65532\[r15\]\.l, r15
