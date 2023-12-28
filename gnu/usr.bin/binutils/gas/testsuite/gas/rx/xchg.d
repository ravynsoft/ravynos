#source: ./xchg.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc 43 00                      	xchg	r0, r0
   3:	fc 43 0f                      	xchg	r0, r15
   6:	fc 43 f0                      	xchg	r15, r0
   9:	fc 43 ff                      	xchg	r15, r15
   c:	fc 40 00                      	xchg	\[r0\]\.ub, r0
   f:	fc 40 0f                      	xchg	\[r0\]\.ub, r15
  12:	06 20 10 00                   	xchg	\[r0\]\.b, r0
  16:	06 20 10 0f                   	xchg	\[r0\]\.b, r15
  1a:	06 e0 10 00                   	xchg	\[r0\]\.uw, r0
  1e:	06 e0 10 0f                   	xchg	\[r0\]\.uw, r15
  22:	06 60 10 00                   	xchg	\[r0\]\.w, r0
  26:	06 60 10 0f                   	xchg	\[r0\]\.w, r15
  2a:	06 a0 10 00                   	xchg	\[r0\]\.l, r0
  2e:	06 a0 10 0f                   	xchg	\[r0\]\.l, r15
  32:	fc 40 f0                      	xchg	\[r15\]\.ub, r0
  35:	fc 40 ff                      	xchg	\[r15\]\.ub, r15
  38:	06 20 10 f0                   	xchg	\[r15\]\.b, r0
  3c:	06 20 10 ff                   	xchg	\[r15\]\.b, r15
  40:	06 e0 10 f0                   	xchg	\[r15\]\.uw, r0
  44:	06 e0 10 ff                   	xchg	\[r15\]\.uw, r15
  48:	06 60 10 f0                   	xchg	\[r15\]\.w, r0
  4c:	06 60 10 ff                   	xchg	\[r15\]\.w, r15
  50:	06 a0 10 f0                   	xchg	\[r15\]\.l, r0
  54:	06 a0 10 ff                   	xchg	\[r15\]\.l, r15
  58:	fc 41 00 fc                   	xchg	252\[r0\]\.ub, r0
  5c:	fc 41 0f fc                   	xchg	252\[r0\]\.ub, r15
  60:	06 21 10 00 fc                	xchg	252\[r0\]\.b, r0
  65:	06 21 10 0f fc                	xchg	252\[r0\]\.b, r15
  6a:	06 e1 10 00 7e                	xchg	252\[r0\]\.uw, r0
  6f:	06 e1 10 0f 7e                	xchg	252\[r0\]\.uw, r15
  74:	06 61 10 00 7e                	xchg	252\[r0\]\.w, r0
  79:	06 61 10 0f 7e                	xchg	252\[r0\]\.w, r15
  7e:	06 a1 10 00 3f                	xchg	252\[r0\]\.l, r0
  83:	06 a1 10 0f 3f                	xchg	252\[r0\]\.l, r15
  88:	fc 41 f0 fc                   	xchg	252\[r15\]\.ub, r0
  8c:	fc 41 ff fc                   	xchg	252\[r15\]\.ub, r15
  90:	06 21 10 f0 fc                	xchg	252\[r15\]\.b, r0
  95:	06 21 10 ff fc                	xchg	252\[r15\]\.b, r15
  9a:	06 e1 10 f0 7e                	xchg	252\[r15\]\.uw, r0
  9f:	06 e1 10 ff 7e                	xchg	252\[r15\]\.uw, r15
  a4:	06 61 10 f0 7e                	xchg	252\[r15\]\.w, r0
  a9:	06 61 10 ff 7e                	xchg	252\[r15\]\.w, r15
  ae:	06 a1 10 f0 3f                	xchg	252\[r15\]\.l, r0
  b3:	06 a1 10 ff 3f                	xchg	252\[r15\]\.l, r15
  b8:	fc 42 00 fc ff                	xchg	65532\[r0\]\.ub, r0
  bd:	fc 42 0f fc ff                	xchg	65532\[r0\]\.ub, r15
  c2:	06 22 10 00 fc ff             	xchg	65532\[r0\]\.b, r0
  c8:	06 22 10 0f fc ff             	xchg	65532\[r0\]\.b, r15
  ce:	06 e2 10 00 fe 7f             	xchg	65532\[r0\]\.uw, r0
  d4:	06 e2 10 0f fe 7f             	xchg	65532\[r0\]\.uw, r15
  da:	06 62 10 00 fe 7f             	xchg	65532\[r0\]\.w, r0
  e0:	06 62 10 0f fe 7f             	xchg	65532\[r0\]\.w, r15
  e6:	06 a2 10 00 ff 3f             	xchg	65532\[r0\]\.l, r0
  ec:	06 a2 10 0f ff 3f             	xchg	65532\[r0\]\.l, r15
  f2:	fc 42 f0 fc ff                	xchg	65532\[r15\]\.ub, r0
  f7:	fc 42 ff fc ff                	xchg	65532\[r15\]\.ub, r15
  fc:	06 22 10 f0 fc ff             	xchg	65532\[r15\]\.b, r0
 102:	06 22 10 ff fc ff             	xchg	65532\[r15\]\.b, r15
 108:	06 e2 10 f0 fe 7f             	xchg	65532\[r15\]\.uw, r0
 10e:	06 e2 10 ff fe 7f             	xchg	65532\[r15\]\.uw, r15
 114:	06 62 10 f0 fe 7f             	xchg	65532\[r15\]\.w, r0
 11a:	06 62 10 ff fe 7f             	xchg	65532\[r15\]\.w, r15
 120:	06 a2 10 f0 ff 3f             	xchg	65532\[r15\]\.l, r0
 126:	06 a2 10 ff ff 3f             	xchg	65532\[r15\]\.l, r15
