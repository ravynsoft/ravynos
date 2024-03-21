#source: ./emulu.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 70 80                   	emulu	#-128, r0
   4:	fd 74 7e 80                   	emulu	#-128, r14
   8:	fd 74 70 7f                   	emulu	#127, r0
   c:	fd 74 7e 7f                   	emulu	#127, r14
  10:	fd 78 70 00 80                	emulu	#0xffff8000, r0
  15:	fd 78 7e 00 80                	emulu	#0xffff8000, r14
  1a:	fd 7c 70 00 80 00             	emulu	#0x8000, r0
  20:	fd 7c 7e 00 80 00             	emulu	#0x8000, r14
  26:	fd 7c 70 00 00 80             	emulu	#0xff800000, r0
  2c:	fd 7c 7e 00 00 80             	emulu	#0xff800000, r14
  32:	fd 7c 70 ff ff 7f             	emulu	#0x7fffff, r0
  38:	fd 7c 7e ff ff 7f             	emulu	#0x7fffff, r14
  3e:	fd 70 70 00 00 00 80          	emulu	#0x80000000, r0
  45:	fd 70 7e 00 00 00 80          	emulu	#0x80000000, r14
  4c:	fd 70 70 ff ff ff 7f          	emulu	#0x7fffffff, r0
  53:	fd 70 7e ff ff ff 7f          	emulu	#0x7fffffff, r14
  5a:	fc 1f 00                      	emulu	r0, r0
  5d:	fc 1f 0e                      	emulu	r0, r14
  60:	fc 1f f0                      	emulu	r15, r0
  63:	fc 1f fe                      	emulu	r15, r14
  66:	fc 1c 00                      	emulu	\[r0\]\.ub, r0
  69:	fc 1c 0e                      	emulu	\[r0\]\.ub, r14
  6c:	06 20 07 00                   	emulu	\[r0\]\.b, r0
  70:	06 20 07 0e                   	emulu	\[r0\]\.b, r14
  74:	06 e0 07 00                   	emulu	\[r0\]\.uw, r0
  78:	06 e0 07 0e                   	emulu	\[r0\]\.uw, r14
  7c:	06 60 07 00                   	emulu	\[r0\]\.w, r0
  80:	06 60 07 0e                   	emulu	\[r0\]\.w, r14
  84:	06 a0 07 00                   	emulu	\[r0\]\.l, r0
  88:	06 a0 07 0e                   	emulu	\[r0\]\.l, r14
  8c:	fc 1c f0                      	emulu	\[r15\]\.ub, r0
  8f:	fc 1c fe                      	emulu	\[r15\]\.ub, r14
  92:	06 20 07 f0                   	emulu	\[r15\]\.b, r0
  96:	06 20 07 fe                   	emulu	\[r15\]\.b, r14
  9a:	06 e0 07 f0                   	emulu	\[r15\]\.uw, r0
  9e:	06 e0 07 fe                   	emulu	\[r15\]\.uw, r14
  a2:	06 60 07 f0                   	emulu	\[r15\]\.w, r0
  a6:	06 60 07 fe                   	emulu	\[r15\]\.w, r14
  aa:	06 a0 07 f0                   	emulu	\[r15\]\.l, r0
  ae:	06 a0 07 fe                   	emulu	\[r15\]\.l, r14
  b2:	fc 1d 00 fc                   	emulu	252\[r0\]\.ub, r0
  b6:	fc 1d 0e fc                   	emulu	252\[r0\]\.ub, r14
  ba:	06 21 07 00 fc                	emulu	252\[r0\]\.b, r0
  bf:	06 21 07 0e fc                	emulu	252\[r0\]\.b, r14
  c4:	06 e1 07 00 7e                	emulu	252\[r0\]\.uw, r0
  c9:	06 e1 07 0e 7e                	emulu	252\[r0\]\.uw, r14
  ce:	06 61 07 00 7e                	emulu	252\[r0\]\.w, r0
  d3:	06 61 07 0e 7e                	emulu	252\[r0\]\.w, r14
  d8:	06 a1 07 00 3f                	emulu	252\[r0\]\.l, r0
  dd:	06 a1 07 0e 3f                	emulu	252\[r0\]\.l, r14
  e2:	fc 1d f0 fc                   	emulu	252\[r15\]\.ub, r0
  e6:	fc 1d fe fc                   	emulu	252\[r15\]\.ub, r14
  ea:	06 21 07 f0 fc                	emulu	252\[r15\]\.b, r0
  ef:	06 21 07 fe fc                	emulu	252\[r15\]\.b, r14
  f4:	06 e1 07 f0 7e                	emulu	252\[r15\]\.uw, r0
  f9:	06 e1 07 fe 7e                	emulu	252\[r15\]\.uw, r14
  fe:	06 61 07 f0 7e                	emulu	252\[r15\]\.w, r0
 103:	06 61 07 fe 7e                	emulu	252\[r15\]\.w, r14
 108:	06 a1 07 f0 3f                	emulu	252\[r15\]\.l, r0
 10d:	06 a1 07 fe 3f                	emulu	252\[r15\]\.l, r14
 112:	fc 1e 00 fc ff                	emulu	65532\[r0\]\.ub, r0
 117:	fc 1e 0e fc ff                	emulu	65532\[r0\]\.ub, r14
 11c:	06 22 07 00 fc ff             	emulu	65532\[r0\]\.b, r0
 122:	06 22 07 0e fc ff             	emulu	65532\[r0\]\.b, r14
 128:	06 e2 07 00 fe 7f             	emulu	65532\[r0\]\.uw, r0
 12e:	06 e2 07 0e fe 7f             	emulu	65532\[r0\]\.uw, r14
 134:	06 62 07 00 fe 7f             	emulu	65532\[r0\]\.w, r0
 13a:	06 62 07 0e fe 7f             	emulu	65532\[r0\]\.w, r14
 140:	06 a2 07 00 ff 3f             	emulu	65532\[r0\]\.l, r0
 146:	06 a2 07 0e ff 3f             	emulu	65532\[r0\]\.l, r14
 14c:	fc 1e f0 fc ff                	emulu	65532\[r15\]\.ub, r0
 151:	fc 1e fe fc ff                	emulu	65532\[r15\]\.ub, r14
 156:	06 22 07 f0 fc ff             	emulu	65532\[r15\]\.b, r0
 15c:	06 22 07 fe fc ff             	emulu	65532\[r15\]\.b, r14
 162:	06 e2 07 f0 fe 7f             	emulu	65532\[r15\]\.uw, r0
 168:	06 e2 07 fe fe 7f             	emulu	65532\[r15\]\.uw, r14
 16e:	06 62 07 f0 fe 7f             	emulu	65532\[r15\]\.w, r0
 174:	06 62 07 fe fe 7f             	emulu	65532\[r15\]\.w, r14
 17a:	06 a2 07 f0 ff 3f             	emulu	65532\[r15\]\.l, r0
 180:	06 a2 07 fe ff 3f             	emulu	65532\[r15\]\.l, r14
