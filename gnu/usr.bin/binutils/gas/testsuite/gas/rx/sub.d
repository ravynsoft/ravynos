#source: ./sub.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	60 00                         	sub	#0, r0
   2:	60 0f                         	sub	#0, r15
   4:	60 f0                         	sub	#15, r0
   6:	60 ff                         	sub	#15, r15
   8:	43 00                         	sub	r0, r0
   a:	43 0f                         	sub	r0, r15
   c:	43 f0                         	sub	r15, r0
   e:	43 ff                         	sub	r15, r15
  10:	40 00                         	sub	\[r0\]\.ub, r0
  12:	40 0f                         	sub	\[r0\]\.ub, r15
  14:	06 00 00                      	sub	\[r0\]\.b, r0
  17:	06 00 0f                      	sub	\[r0\]\.b, r15
  1a:	06 c0 00                      	sub	\[r0\]\.uw, r0
  1d:	06 c0 0f                      	sub	\[r0\]\.uw, r15
  20:	06 40 00                      	sub	\[r0\]\.w, r0
  23:	06 40 0f                      	sub	\[r0\]\.w, r15
  26:	06 80 00                      	sub	\[r0\]\.l, r0
  29:	06 80 0f                      	sub	\[r0\]\.l, r15
  2c:	40 f0                         	sub	\[r15\]\.ub, r0
  2e:	40 ff                         	sub	\[r15\]\.ub, r15
  30:	06 00 f0                      	sub	\[r15\]\.b, r0
  33:	06 00 ff                      	sub	\[r15\]\.b, r15
  36:	06 c0 f0                      	sub	\[r15\]\.uw, r0
  39:	06 c0 ff                      	sub	\[r15\]\.uw, r15
  3c:	06 40 f0                      	sub	\[r15\]\.w, r0
  3f:	06 40 ff                      	sub	\[r15\]\.w, r15
  42:	06 80 f0                      	sub	\[r15\]\.l, r0
  45:	06 80 ff                      	sub	\[r15\]\.l, r15
  48:	41 00 fc                      	sub	252\[r0\]\.ub, r0
  4b:	41 0f fc                      	sub	252\[r0\]\.ub, r15
  4e:	06 01 00 fc                   	sub	252\[r0\]\.b, r0
  52:	06 01 0f fc                   	sub	252\[r0\]\.b, r15
  56:	06 c1 00 7e                   	sub	252\[r0\]\.uw, r0
  5a:	06 c1 0f 7e                   	sub	252\[r0\]\.uw, r15
  5e:	06 41 00 7e                   	sub	252\[r0\]\.w, r0
  62:	06 41 0f 7e                   	sub	252\[r0\]\.w, r15
  66:	06 81 00 3f                   	sub	252\[r0\]\.l, r0
  6a:	06 81 0f 3f                   	sub	252\[r0\]\.l, r15
  6e:	41 f0 fc                      	sub	252\[r15\]\.ub, r0
  71:	41 ff fc                      	sub	252\[r15\]\.ub, r15
  74:	06 01 f0 fc                   	sub	252\[r15\]\.b, r0
  78:	06 01 ff fc                   	sub	252\[r15\]\.b, r15
  7c:	06 c1 f0 7e                   	sub	252\[r15\]\.uw, r0
  80:	06 c1 ff 7e                   	sub	252\[r15\]\.uw, r15
  84:	06 41 f0 7e                   	sub	252\[r15\]\.w, r0
  88:	06 41 ff 7e                   	sub	252\[r15\]\.w, r15
  8c:	06 81 f0 3f                   	sub	252\[r15\]\.l, r0
  90:	06 81 ff 3f                   	sub	252\[r15\]\.l, r15
  94:	42 00 fc ff                   	sub	65532\[r0\]\.ub, r0
  98:	42 0f fc ff                   	sub	65532\[r0\]\.ub, r15
  9c:	06 02 00 fc ff                	sub	65532\[r0\]\.b, r0
  a1:	06 02 0f fc ff                	sub	65532\[r0\]\.b, r15
  a6:	06 c2 00 fe 7f                	sub	65532\[r0\]\.uw, r0
  ab:	06 c2 0f fe 7f                	sub	65532\[r0\]\.uw, r15
  b0:	06 42 00 fe 7f                	sub	65532\[r0\]\.w, r0
  b5:	06 42 0f fe 7f                	sub	65532\[r0\]\.w, r15
  ba:	06 82 00 ff 3f                	sub	65532\[r0\]\.l, r0
  bf:	06 82 0f ff 3f                	sub	65532\[r0\]\.l, r15
  c4:	42 f0 fc ff                   	sub	65532\[r15\]\.ub, r0
  c8:	42 ff fc ff                   	sub	65532\[r15\]\.ub, r15
  cc:	06 02 f0 fc ff                	sub	65532\[r15\]\.b, r0
  d1:	06 02 ff fc ff                	sub	65532\[r15\]\.b, r15
  d6:	06 c2 f0 fe 7f                	sub	65532\[r15\]\.uw, r0
  db:	06 c2 ff fe 7f                	sub	65532\[r15\]\.uw, r15
  e0:	06 42 f0 fe 7f                	sub	65532\[r15\]\.w, r0
  e5:	06 42 ff fe 7f                	sub	65532\[r15\]\.w, r15
  ea:	06 82 f0 ff 3f                	sub	65532\[r15\]\.l, r0
  ef:	06 82 ff ff 3f                	sub	65532\[r15\]\.l, r15
  f4:	ff 00 00                      	sub	r0, r0, r0
  f7:	ff 0f 00                      	sub	r0, r0, r15
  fa:	ff 00 0f                      	sub	r0, r15, r0
  fd:	ff 0f 0f                      	sub	r0, r15, r15
 100:	ff 00 f0                      	sub	r15, r0, r0
 103:	ff 0f f0                      	sub	r15, r0, r15
 106:	ff 00 ff                      	sub	r15, r15, r0
 109:	ff 0f ff                      	sub	r15, r15, r15
