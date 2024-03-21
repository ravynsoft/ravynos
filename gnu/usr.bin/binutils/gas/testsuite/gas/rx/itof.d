#source: ./itof.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 47 00                      	itof	r0, r0
   3:	fc 47 0f                      	itof	r0, r15
   6:	fc 47 f0                      	itof	r15, r0
   9:	fc 47 ff                      	itof	r15, r15
   c:	fc 44 00                      	itof	\[r0\]\.ub, r0
   f:	fc 44 0f                      	itof	\[r0\]\.ub, r15
  12:	06 20 11 00                   	itof	\[r0\]\.b, r0
  16:	06 20 11 0f                   	itof	\[r0\]\.b, r15
  1a:	06 e0 11 00                   	itof	\[r0\]\.uw, r0
  1e:	06 e0 11 0f                   	itof	\[r0\]\.uw, r15
  22:	06 60 11 00                   	itof	\[r0\]\.w, r0
  26:	06 60 11 0f                   	itof	\[r0\]\.w, r15
  2a:	06 a0 11 00                   	itof	\[r0\]\.l, r0
  2e:	06 a0 11 0f                   	itof	\[r0\]\.l, r15
  32:	fc 44 f0                      	itof	\[r15\]\.ub, r0
  35:	fc 44 ff                      	itof	\[r15\]\.ub, r15
  38:	06 20 11 f0                   	itof	\[r15\]\.b, r0
  3c:	06 20 11 ff                   	itof	\[r15\]\.b, r15
  40:	06 e0 11 f0                   	itof	\[r15\]\.uw, r0
  44:	06 e0 11 ff                   	itof	\[r15\]\.uw, r15
  48:	06 60 11 f0                   	itof	\[r15\]\.w, r0
  4c:	06 60 11 ff                   	itof	\[r15\]\.w, r15
  50:	06 a0 11 f0                   	itof	\[r15\]\.l, r0
  54:	06 a0 11 ff                   	itof	\[r15\]\.l, r15
  58:	fc 45 00 fc                   	itof	252\[r0\]\.ub, r0
  5c:	fc 45 0f fc                   	itof	252\[r0\]\.ub, r15
  60:	06 21 11 00 fc                	itof	252\[r0\]\.b, r0
  65:	06 21 11 0f fc                	itof	252\[r0\]\.b, r15
  6a:	06 e1 11 00 7e                	itof	252\[r0\]\.uw, r0
  6f:	06 e1 11 0f 7e                	itof	252\[r0\]\.uw, r15
  74:	06 61 11 00 7e                	itof	252\[r0\]\.w, r0
  79:	06 61 11 0f 7e                	itof	252\[r0\]\.w, r15
  7e:	06 a1 11 00 3f                	itof	252\[r0\]\.l, r0
  83:	06 a1 11 0f 3f                	itof	252\[r0\]\.l, r15
  88:	fc 45 f0 fc                   	itof	252\[r15\]\.ub, r0
  8c:	fc 45 ff fc                   	itof	252\[r15\]\.ub, r15
  90:	06 21 11 f0 fc                	itof	252\[r15\]\.b, r0
  95:	06 21 11 ff fc                	itof	252\[r15\]\.b, r15
  9a:	06 e1 11 f0 7e                	itof	252\[r15\]\.uw, r0
  9f:	06 e1 11 ff 7e                	itof	252\[r15\]\.uw, r15
  a4:	06 61 11 f0 7e                	itof	252\[r15\]\.w, r0
  a9:	06 61 11 ff 7e                	itof	252\[r15\]\.w, r15
  ae:	06 a1 11 f0 3f                	itof	252\[r15\]\.l, r0
  b3:	06 a1 11 ff 3f                	itof	252\[r15\]\.l, r15
  b8:	fc 46 00 fc ff                	itof	65532\[r0\]\.ub, r0
  bd:	fc 46 0f fc ff                	itof	65532\[r0\]\.ub, r15
  c2:	06 22 11 00 fc ff             	itof	65532\[r0\]\.b, r0
  c8:	06 22 11 0f fc ff             	itof	65532\[r0\]\.b, r15
  ce:	06 e2 11 00 fe 7f             	itof	65532\[r0\]\.uw, r0
  d4:	06 e2 11 0f fe 7f             	itof	65532\[r0\]\.uw, r15
  da:	06 62 11 00 fe 7f             	itof	65532\[r0\]\.w, r0
  e0:	06 62 11 0f fe 7f             	itof	65532\[r0\]\.w, r15
  e6:	06 a2 11 00 ff 3f             	itof	65532\[r0\]\.l, r0
  ec:	06 a2 11 0f ff 3f             	itof	65532\[r0\]\.l, r15
  f2:	fc 46 f0 fc ff                	itof	65532\[r15\]\.ub, r0
  f7:	fc 46 ff fc ff                	itof	65532\[r15\]\.ub, r15
  fc:	06 22 11 f0 fc ff             	itof	65532\[r15\]\.b, r0
 102:	06 22 11 ff fc ff             	itof	65532\[r15\]\.b, r15
 108:	06 e2 11 f0 fe 7f             	itof	65532\[r15\]\.uw, r0
 10e:	06 e2 11 ff fe 7f             	itof	65532\[r15\]\.uw, r15
 114:	06 62 11 f0 fe 7f             	itof	65532\[r15\]\.w, r0
 11a:	06 62 11 ff fe 7f             	itof	65532\[r15\]\.w, r15
 120:	06 a2 11 f0 ff 3f             	itof	65532\[r15\]\.l, r0
 126:	06 a2 11 ff ff 3f             	itof	65532\[r15\]\.l, r15
