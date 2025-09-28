#source: ./xor.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 d0 80                   	xor	#-128, r0
   4:	fd 74 df 80                   	xor	#-128, r15
   8:	fd 74 d0 7f                   	xor	#127, r0
   c:	fd 74 df 7f                   	xor	#127, r15
  10:	fd 78 d0 00 80                	xor	#0xffff8000, r0
  15:	fd 78 df 00 80                	xor	#0xffff8000, r15
  1a:	fd 7c d0 00 80 00             	xor	#0x8000, r0
  20:	fd 7c df 00 80 00             	xor	#0x8000, r15
  26:	fd 7c d0 00 00 80             	xor	#0xff800000, r0
  2c:	fd 7c df 00 00 80             	xor	#0xff800000, r15
  32:	fd 7c d0 ff ff 7f             	xor	#0x7fffff, r0
  38:	fd 7c df ff ff 7f             	xor	#0x7fffff, r15
  3e:	fd 70 d0 00 00 00 80          	xor	#0x80000000, r0
  45:	fd 70 df 00 00 00 80          	xor	#0x80000000, r15
  4c:	fd 70 d0 ff ff ff 7f          	xor	#0x7fffffff, r0
  53:	fd 70 df ff ff ff 7f          	xor	#0x7fffffff, r15
  5a:	fc 37 00                      	xor	r0, r0
  5d:	fc 37 0f                      	xor	r0, r15
  60:	fc 37 f0                      	xor	r15, r0
  63:	fc 37 ff                      	xor	r15, r15
  66:	fc 34 00                      	xor	\[r0\]\.ub, r0
  69:	fc 34 0f                      	xor	\[r0\]\.ub, r15
  6c:	06 20 0d 00                   	xor	\[r0\]\.b, r0
  70:	06 20 0d 0f                   	xor	\[r0\]\.b, r15
  74:	06 e0 0d 00                   	xor	\[r0\]\.uw, r0
  78:	06 e0 0d 0f                   	xor	\[r0\]\.uw, r15
  7c:	06 60 0d 00                   	xor	\[r0\]\.w, r0
  80:	06 60 0d 0f                   	xor	\[r0\]\.w, r15
  84:	06 a0 0d 00                   	xor	\[r0\]\.l, r0
  88:	06 a0 0d 0f                   	xor	\[r0\]\.l, r15
  8c:	fc 34 f0                      	xor	\[r15\]\.ub, r0
  8f:	fc 34 ff                      	xor	\[r15\]\.ub, r15
  92:	06 20 0d f0                   	xor	\[r15\]\.b, r0
  96:	06 20 0d ff                   	xor	\[r15\]\.b, r15
  9a:	06 e0 0d f0                   	xor	\[r15\]\.uw, r0
  9e:	06 e0 0d ff                   	xor	\[r15\]\.uw, r15
  a2:	06 60 0d f0                   	xor	\[r15\]\.w, r0
  a6:	06 60 0d ff                   	xor	\[r15\]\.w, r15
  aa:	06 a0 0d f0                   	xor	\[r15\]\.l, r0
  ae:	06 a0 0d ff                   	xor	\[r15\]\.l, r15
  b2:	fc 35 00 fc                   	xor	252\[r0\]\.ub, r0
  b6:	fc 35 0f fc                   	xor	252\[r0\]\.ub, r15
  ba:	06 21 0d 00 fc                	xor	252\[r0\]\.b, r0
  bf:	06 21 0d 0f fc                	xor	252\[r0\]\.b, r15
  c4:	06 e1 0d 00 7e                	xor	252\[r0\]\.uw, r0
  c9:	06 e1 0d 0f 7e                	xor	252\[r0\]\.uw, r15
  ce:	06 61 0d 00 7e                	xor	252\[r0\]\.w, r0
  d3:	06 61 0d 0f 7e                	xor	252\[r0\]\.w, r15
  d8:	06 a1 0d 00 3f                	xor	252\[r0\]\.l, r0
  dd:	06 a1 0d 0f 3f                	xor	252\[r0\]\.l, r15
  e2:	fc 35 f0 fc                   	xor	252\[r15\]\.ub, r0
  e6:	fc 35 ff fc                   	xor	252\[r15\]\.ub, r15
  ea:	06 21 0d f0 fc                	xor	252\[r15\]\.b, r0
  ef:	06 21 0d ff fc                	xor	252\[r15\]\.b, r15
  f4:	06 e1 0d f0 7e                	xor	252\[r15\]\.uw, r0
  f9:	06 e1 0d ff 7e                	xor	252\[r15\]\.uw, r15
  fe:	06 61 0d f0 7e                	xor	252\[r15\]\.w, r0
 103:	06 61 0d ff 7e                	xor	252\[r15\]\.w, r15
 108:	06 a1 0d f0 3f                	xor	252\[r15\]\.l, r0
 10d:	06 a1 0d ff 3f                	xor	252\[r15\]\.l, r15
 112:	fc 36 00 fc ff                	xor	65532\[r0\]\.ub, r0
 117:	fc 36 0f fc ff                	xor	65532\[r0\]\.ub, r15
 11c:	06 22 0d 00 fc ff             	xor	65532\[r0\]\.b, r0
 122:	06 22 0d 0f fc ff             	xor	65532\[r0\]\.b, r15
 128:	06 e2 0d 00 fe 7f             	xor	65532\[r0\]\.uw, r0
 12e:	06 e2 0d 0f fe 7f             	xor	65532\[r0\]\.uw, r15
 134:	06 62 0d 00 fe 7f             	xor	65532\[r0\]\.w, r0
 13a:	06 62 0d 0f fe 7f             	xor	65532\[r0\]\.w, r15
 140:	06 a2 0d 00 ff 3f             	xor	65532\[r0\]\.l, r0
 146:	06 a2 0d 0f ff 3f             	xor	65532\[r0\]\.l, r15
 14c:	fc 36 f0 fc ff                	xor	65532\[r15\]\.ub, r0
 151:	fc 36 ff fc ff                	xor	65532\[r15\]\.ub, r15
 156:	06 22 0d f0 fc ff             	xor	65532\[r15\]\.b, r0
 15c:	06 22 0d ff fc ff             	xor	65532\[r15\]\.b, r15
 162:	06 e2 0d f0 fe 7f             	xor	65532\[r15\]\.uw, r0
 168:	06 e2 0d ff fe 7f             	xor	65532\[r15\]\.uw, r15
 16e:	06 62 0d f0 fe 7f             	xor	65532\[r15\]\.w, r0
 174:	06 62 0d ff fe 7f             	xor	65532\[r15\]\.w, r15
 17a:	06 a2 0d f0 ff 3f             	xor	65532\[r15\]\.l, r0
 180:	06 a2 0d ff ff 3f             	xor	65532\[r15\]\.l, r15
 186:	ff 60 00                      	xor	r0, r0, r0
 189:	ff 6f 00                      	xor	r0, r0, r15
 18c:	ff 60 0f                      	xor	r0, r15, r0
 18f:	ff 6f 0f                      	xor	r0, r15, r15
 192:	ff 60 f0                      	xor	r15, r0, r0
 195:	ff 6f f0                      	xor	r15, r0, r15
 198:	ff 60 ff                      	xor	r15, r15, r0
 19b:	ff 6f ff                      	xor	r15, r15, r15
