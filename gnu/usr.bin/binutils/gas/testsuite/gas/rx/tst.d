#source: ./tst.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 c0 80                   	tst	#-128, r0
   4:	fd 74 cf 80                   	tst	#-128, r15
   8:	fd 74 c0 7f                   	tst	#127, r0
   c:	fd 74 cf 7f                   	tst	#127, r15
  10:	fd 78 c0 00 80                	tst	#0xffff8000, r0
  15:	fd 78 cf 00 80                	tst	#0xffff8000, r15
  1a:	fd 7c c0 00 80 00             	tst	#0x8000, r0
  20:	fd 7c cf 00 80 00             	tst	#0x8000, r15
  26:	fd 7c c0 00 00 80             	tst	#0xff800000, r0
  2c:	fd 7c cf 00 00 80             	tst	#0xff800000, r15
  32:	fd 7c c0 ff ff 7f             	tst	#0x7fffff, r0
  38:	fd 7c cf ff ff 7f             	tst	#0x7fffff, r15
  3e:	fd 70 c0 00 00 00 80          	tst	#0x80000000, r0
  45:	fd 70 cf 00 00 00 80          	tst	#0x80000000, r15
  4c:	fd 70 c0 ff ff ff 7f          	tst	#0x7fffffff, r0
  53:	fd 70 cf ff ff ff 7f          	tst	#0x7fffffff, r15
  5a:	fc 33 00                      	tst	r0, r0
  5d:	fc 33 0f                      	tst	r0, r15
  60:	fc 33 f0                      	tst	r15, r0
  63:	fc 33 ff                      	tst	r15, r15
  66:	fc 30 00                      	tst	\[r0\]\.ub, r0
  69:	fc 30 0f                      	tst	\[r0\]\.ub, r15
  6c:	06 20 0c 00                   	tst	\[r0\]\.b, r0
  70:	06 20 0c 0f                   	tst	\[r0\]\.b, r15
  74:	06 e0 0c 00                   	tst	\[r0\]\.uw, r0
  78:	06 e0 0c 0f                   	tst	\[r0\]\.uw, r15
  7c:	06 60 0c 00                   	tst	\[r0\]\.w, r0
  80:	06 60 0c 0f                   	tst	\[r0\]\.w, r15
  84:	06 a0 0c 00                   	tst	\[r0\]\.l, r0
  88:	06 a0 0c 0f                   	tst	\[r0\]\.l, r15
  8c:	fc 30 f0                      	tst	\[r15\]\.ub, r0
  8f:	fc 30 ff                      	tst	\[r15\]\.ub, r15
  92:	06 20 0c f0                   	tst	\[r15\]\.b, r0
  96:	06 20 0c ff                   	tst	\[r15\]\.b, r15
  9a:	06 e0 0c f0                   	tst	\[r15\]\.uw, r0
  9e:	06 e0 0c ff                   	tst	\[r15\]\.uw, r15
  a2:	06 60 0c f0                   	tst	\[r15\]\.w, r0
  a6:	06 60 0c ff                   	tst	\[r15\]\.w, r15
  aa:	06 a0 0c f0                   	tst	\[r15\]\.l, r0
  ae:	06 a0 0c ff                   	tst	\[r15\]\.l, r15
  b2:	fc 31 00 fc                   	tst	252\[r0\]\.ub, r0
  b6:	fc 31 0f fc                   	tst	252\[r0\]\.ub, r15
  ba:	06 21 0c 00 fc                	tst	252\[r0\]\.b, r0
  bf:	06 21 0c 0f fc                	tst	252\[r0\]\.b, r15
  c4:	06 e1 0c 00 7e                	tst	252\[r0\]\.uw, r0
  c9:	06 e1 0c 0f 7e                	tst	252\[r0\]\.uw, r15
  ce:	06 61 0c 00 7e                	tst	252\[r0\]\.w, r0
  d3:	06 61 0c 0f 7e                	tst	252\[r0\]\.w, r15
  d8:	06 a1 0c 00 3f                	tst	252\[r0\]\.l, r0
  dd:	06 a1 0c 0f 3f                	tst	252\[r0\]\.l, r15
  e2:	fc 31 f0 fc                   	tst	252\[r15\]\.ub, r0
  e6:	fc 31 ff fc                   	tst	252\[r15\]\.ub, r15
  ea:	06 21 0c f0 fc                	tst	252\[r15\]\.b, r0
  ef:	06 21 0c ff fc                	tst	252\[r15\]\.b, r15
  f4:	06 e1 0c f0 7e                	tst	252\[r15\]\.uw, r0
  f9:	06 e1 0c ff 7e                	tst	252\[r15\]\.uw, r15
  fe:	06 61 0c f0 7e                	tst	252\[r15\]\.w, r0
 103:	06 61 0c ff 7e                	tst	252\[r15\]\.w, r15
 108:	06 a1 0c f0 3f                	tst	252\[r15\]\.l, r0
 10d:	06 a1 0c ff 3f                	tst	252\[r15\]\.l, r15
 112:	fc 32 00 fc ff                	tst	65532\[r0\]\.ub, r0
 117:	fc 32 0f fc ff                	tst	65532\[r0\]\.ub, r15
 11c:	06 22 0c 00 fc ff             	tst	65532\[r0\]\.b, r0
 122:	06 22 0c 0f fc ff             	tst	65532\[r0\]\.b, r15
 128:	06 e2 0c 00 fe 7f             	tst	65532\[r0\]\.uw, r0
 12e:	06 e2 0c 0f fe 7f             	tst	65532\[r0\]\.uw, r15
 134:	06 62 0c 00 fe 7f             	tst	65532\[r0\]\.w, r0
 13a:	06 62 0c 0f fe 7f             	tst	65532\[r0\]\.w, r15
 140:	06 a2 0c 00 ff 3f             	tst	65532\[r0\]\.l, r0
 146:	06 a2 0c 0f ff 3f             	tst	65532\[r0\]\.l, r15
 14c:	fc 32 f0 fc ff                	tst	65532\[r15\]\.ub, r0
 151:	fc 32 ff fc ff                	tst	65532\[r15\]\.ub, r15
 156:	06 22 0c f0 fc ff             	tst	65532\[r15\]\.b, r0
 15c:	06 22 0c ff fc ff             	tst	65532\[r15\]\.b, r15
 162:	06 e2 0c f0 fe 7f             	tst	65532\[r15\]\.uw, r0
 168:	06 e2 0c ff fe 7f             	tst	65532\[r15\]\.uw, r15
 16e:	06 62 0c f0 fe 7f             	tst	65532\[r15\]\.w, r0
 174:	06 62 0c ff fe 7f             	tst	65532\[r15\]\.w, r15
 17a:	06 a2 0c f0 ff 3f             	tst	65532\[r15\]\.l, r0
 180:	06 a2 0c ff ff 3f             	tst	65532\[r15\]\.l, r15
