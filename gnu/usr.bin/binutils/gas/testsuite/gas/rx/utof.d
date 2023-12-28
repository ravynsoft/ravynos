#source: ./utof.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 57 00                      	utof	r0, r0
   3:	fc 57 0f                      	utof	r0, r15
   6:	fc 57 f0                      	utof	r15, r0
   9:	fc 57 ff                      	utof	r15, r15
   c:	fc 54 00                      	utof	\[r0\]\.ub, r0
   f:	fc 54 0f                      	utof	\[r0\]\.ub, r15
  12:	06 20 15 00                   	utof	\[r0\]\.b, r0
  16:	06 20 15 0f                   	utof	\[r0\]\.b, r15
  1a:	06 e0 15 00                   	utof	\[r0\]\.uw, r0
  1e:	06 e0 15 0f                   	utof	\[r0\]\.uw, r15
  22:	06 60 15 00                   	utof	\[r0\]\.w, r0
  26:	06 60 15 0f                   	utof	\[r0\]\.w, r15
  2a:	06 a0 15 00                   	utof	\[r0\]\.l, r0
  2e:	06 a0 15 0f                   	utof	\[r0\]\.l, r15
  32:	fc 54 f0                      	utof	\[r15\]\.ub, r0
  35:	fc 54 ff                      	utof	\[r15\]\.ub, r15
  38:	06 20 15 f0                   	utof	\[r15\]\.b, r0
  3c:	06 20 15 ff                   	utof	\[r15\]\.b, r15
  40:	06 e0 15 f0                   	utof	\[r15\]\.uw, r0
  44:	06 e0 15 ff                   	utof	\[r15\]\.uw, r15
  48:	06 60 15 f0                   	utof	\[r15\]\.w, r0
  4c:	06 60 15 ff                   	utof	\[r15\]\.w, r15
  50:	06 a0 15 f0                   	utof	\[r15\]\.l, r0
  54:	06 a0 15 ff                   	utof	\[r15\]\.l, r15
  58:	fc 55 00 fc                   	utof	252\[r0\]\.ub, r0
  5c:	fc 55 0f fc                   	utof	252\[r0\]\.ub, r15
  60:	06 21 15 00 fc                	utof	252\[r0\]\.b, r0
  65:	06 21 15 0f fc                	utof	252\[r0\]\.b, r15
  6a:	06 e1 15 00 7e                	utof	252\[r0\]\.uw, r0
  6f:	06 e1 15 0f 7e                	utof	252\[r0\]\.uw, r15
  74:	06 61 15 00 7e                	utof	252\[r0\]\.w, r0
  79:	06 61 15 0f 7e                	utof	252\[r0\]\.w, r15
  7e:	06 a1 15 00 3f                	utof	252\[r0\]\.l, r0
  83:	06 a1 15 0f 3f                	utof	252\[r0\]\.l, r15
  88:	fc 55 f0 fc                   	utof	252\[r15\]\.ub, r0
  8c:	fc 55 ff fc                   	utof	252\[r15\]\.ub, r15
  90:	06 21 15 f0 fc                	utof	252\[r15\]\.b, r0
  95:	06 21 15 ff fc                	utof	252\[r15\]\.b, r15
  9a:	06 e1 15 f0 7e                	utof	252\[r15\]\.uw, r0
  9f:	06 e1 15 ff 7e                	utof	252\[r15\]\.uw, r15
  a4:	06 61 15 f0 7e                	utof	252\[r15\]\.w, r0
  a9:	06 61 15 ff 7e                	utof	252\[r15\]\.w, r15
  ae:	06 a1 15 f0 3f                	utof	252\[r15\]\.l, r0
  b3:	06 a1 15 ff 3f                	utof	252\[r15\]\.l, r15
  b8:	fc 56 00 fc ff                	utof	65532\[r0\]\.ub, r0
  bd:	fc 56 0f fc ff                	utof	65532\[r0\]\.ub, r15
  c2:	06 22 15 00 fc ff             	utof	65532\[r0\]\.b, r0
  c8:	06 22 15 0f fc ff             	utof	65532\[r0\]\.b, r15
  ce:	06 e2 15 00 fe 7f             	utof	65532\[r0\]\.uw, r0
  d4:	06 e2 15 0f fe 7f             	utof	65532\[r0\]\.uw, r15
  da:	06 62 15 00 fe 7f             	utof	65532\[r0\]\.w, r0
  e0:	06 62 15 0f fe 7f             	utof	65532\[r0\]\.w, r15
  e6:	06 a2 15 00 ff 3f             	utof	65532\[r0\]\.l, r0
  ec:	06 a2 15 0f ff 3f             	utof	65532\[r0\]\.l, r15
  f2:	fc 56 f0 fc ff                	utof	65532\[r15\]\.ub, r0
  f7:	fc 56 ff fc ff                	utof	65532\[r15\]\.ub, r15
  fc:	06 22 15 f0 fc ff             	utof	65532\[r15\]\.b, r0
 102:	06 22 15 ff fc ff             	utof	65532\[r15\]\.b, r15
 108:	06 e2 15 f0 fe 7f             	utof	65532\[r15\]\.uw, r0
 10e:	06 e2 15 ff fe 7f             	utof	65532\[r15\]\.uw, r15
 114:	06 62 15 f0 fe 7f             	utof	65532\[r15\]\.w, r0
 11a:	06 62 15 ff fe 7f             	utof	65532\[r15\]\.w, r15
 120:	06 a2 15 f0 ff 3f             	utof	65532\[r15\]\.l, r0
 126:	06 a2 15 ff ff 3f             	utof	65532\[r15\]\.l, r15
