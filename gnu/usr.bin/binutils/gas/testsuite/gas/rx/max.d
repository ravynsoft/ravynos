#source: ./max.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 74 40 80                   	max	#-128, r0
   4:	fd 74 4f 80                   	max	#-128, r15
   8:	fd 74 40 7f                   	max	#127, r0
   c:	fd 74 4f 7f                   	max	#127, r15
  10:	fd 78 40 00 80                	max	#0xffff8000, r0
  15:	fd 78 4f 00 80                	max	#0xffff8000, r15
  1a:	fd 7c 40 00 80 00             	max	#0x8000, r0
  20:	fd 7c 4f 00 80 00             	max	#0x8000, r15
  26:	fd 7c 40 00 00 80             	max	#0xff800000, r0
  2c:	fd 7c 4f 00 00 80             	max	#0xff800000, r15
  32:	fd 7c 40 ff ff 7f             	max	#0x7fffff, r0
  38:	fd 7c 4f ff ff 7f             	max	#0x7fffff, r15
  3e:	fd 70 40 00 00 00 80          	nop	; max	#0x80000000, r0
  45:	fd 70 4f 00 00 00 80          	max	#0x80000000, r15
  4c:	fd 70 40 ff ff ff 7f          	max	#0x7fffffff, r0
  53:	fd 70 4f ff ff ff 7f          	max	#0x7fffffff, r15
  5a:	fc 13 10                      	max	r1, r0
  5d:	fc 13 1f                      	max	r1, r15
  60:	fc 13 f0                      	max	r15, r0
  63:	fc 13 ff                      	max	r15, r15
  66:	fc 10 00                      	max	\[r0\]\.ub, r0
  69:	fc 10 0f                      	max	\[r0\]\.ub, r15
  6c:	06 20 04 00                   	max	\[r0\]\.b, r0
  70:	06 20 04 0f                   	max	\[r0\]\.b, r15
  74:	06 e0 04 00                   	max	\[r0\]\.uw, r0
  78:	06 e0 04 0f                   	max	\[r0\]\.uw, r15
  7c:	06 60 04 00                   	max	\[r0\]\.w, r0
  80:	06 60 04 0f                   	max	\[r0\]\.w, r15
  84:	06 a0 04 00                   	max	\[r0\]\.l, r0
  88:	06 a0 04 0f                   	max	\[r0\]\.l, r15
  8c:	fc 10 f0                      	max	\[r15\]\.ub, r0
  8f:	fc 10 ff                      	max	\[r15\]\.ub, r15
  92:	06 20 04 f0                   	max	\[r15\]\.b, r0
  96:	06 20 04 ff                   	max	\[r15\]\.b, r15
  9a:	06 e0 04 f0                   	max	\[r15\]\.uw, r0
  9e:	06 e0 04 ff                   	max	\[r15\]\.uw, r15
  a2:	06 60 04 f0                   	max	\[r15\]\.w, r0
  a6:	06 60 04 ff                   	max	\[r15\]\.w, r15
  aa:	06 a0 04 f0                   	max	\[r15\]\.l, r0
  ae:	06 a0 04 ff                   	max	\[r15\]\.l, r15
  b2:	fc 11 00 fc                   	max	252\[r0\]\.ub, r0
  b6:	fc 11 0f fc                   	max	252\[r0\]\.ub, r15
  ba:	06 21 04 00 fc                	max	252\[r0\]\.b, r0
  bf:	06 21 04 0f fc                	max	252\[r0\]\.b, r15
  c4:	06 e1 04 00 7e                	max	252\[r0\]\.uw, r0
  c9:	06 e1 04 0f 7e                	max	252\[r0\]\.uw, r15
  ce:	06 61 04 00 7e                	max	252\[r0\]\.w, r0
  d3:	06 61 04 0f 7e                	max	252\[r0\]\.w, r15
  d8:	06 a1 04 00 3f                	max	252\[r0\]\.l, r0
  dd:	06 a1 04 0f 3f                	max	252\[r0\]\.l, r15
  e2:	fc 11 f0 fc                   	max	252\[r15\]\.ub, r0
  e6:	fc 11 ff fc                   	max	252\[r15\]\.ub, r15
  ea:	06 21 04 f0 fc                	max	252\[r15\]\.b, r0
  ef:	06 21 04 ff fc                	max	252\[r15\]\.b, r15
  f4:	06 e1 04 f0 7e                	max	252\[r15\]\.uw, r0
  f9:	06 e1 04 ff 7e                	max	252\[r15\]\.uw, r15
  fe:	06 61 04 f0 7e                	max	252\[r15\]\.w, r0
 103:	06 61 04 ff 7e                	max	252\[r15\]\.w, r15
 108:	06 a1 04 f0 3f                	max	252\[r15\]\.l, r0
 10d:	06 a1 04 ff 3f                	max	252\[r15\]\.l, r15
 112:	fc 12 00 fc ff                	max	65532\[r0\]\.ub, r0
 117:	fc 12 0f fc ff                	max	65532\[r0\]\.ub, r15
 11c:	06 22 04 00 fc ff             	max	65532\[r0\]\.b, r0
 122:	06 22 04 0f fc ff             	max	65532\[r0\]\.b, r15
 128:	06 e2 04 00 fe 7f             	max	65532\[r0\]\.uw, r0
 12e:	06 e2 04 0f fe 7f             	max	65532\[r0\]\.uw, r15
 134:	06 62 04 00 fe 7f             	max	65532\[r0\]\.w, r0
 13a:	06 62 04 0f fe 7f             	max	65532\[r0\]\.w, r15
 140:	06 a2 04 00 ff 3f             	max	65532\[r0\]\.l, r0
 146:	06 a2 04 0f ff 3f             	max	65532\[r0\]\.l, r15
 14c:	fc 12 f0 fc ff                	max	65532\[r15\]\.ub, r0
 151:	fc 12 ff fc ff                	max	65532\[r15\]\.ub, r15
 156:	06 22 04 f0 fc ff             	max	65532\[r15\]\.b, r0
 15c:	06 22 04 ff fc ff             	max	65532\[r15\]\.b, r15
 162:	06 e2 04 f0 fe 7f             	max	65532\[r15\]\.uw, r0
 168:	06 e2 04 ff fe 7f             	max	65532\[r15\]\.uw, r15
 16e:	06 62 04 f0 fe 7f             	max	65532\[r15\]\.w, r0
 174:	06 62 04 ff fe 7f             	max	65532\[r15\]\.w, r15
 17a:	06 a2 04 f0 ff 3f             	max	65532\[r15\]\.l, r0
 180:	06 a2 04 ff ff 3f             	max	65532\[r15\]\.l, r15
