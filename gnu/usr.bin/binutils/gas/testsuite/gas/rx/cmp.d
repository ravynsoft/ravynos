#source: ./cmp.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	61 00                         	cmp	#0, r0
   2:	61 0f                         	cmp	#0, r15
   4:	61 f0                         	cmp	#15, r0
   6:	61 ff                         	cmp	#15, r15
   8:	61 00                         	cmp	#0, r0
   a:	61 0f                         	cmp	#0, r15
   c:	75 50 ff                      	cmp	#255, r0
   f:	75 5f ff                      	cmp	#255, r15
  12:	75 00 80                      	cmp	#-128, r0
  15:	75 0f 80                      	cmp	#-128, r15
  18:	75 50 7f                      	cmp	#127, r0
  1b:	75 5f 7f                      	cmp	#127, r15
  1e:	76 00 00 80                   	cmp	#0xffff8000, r0
  22:	76 0f 00 80                   	cmp	#0xffff8000, r15
  26:	77 00 00 80 00                	cmp	#0x8000, r0
  2b:	77 0f 00 80 00                	cmp	#0x8000, r15
  30:	77 00 00 00 80                	cmp	#0xff800000, r0
  35:	77 0f 00 00 80                	cmp	#0xff800000, r15
  3a:	77 00 ff ff 7f                	cmp	#0x7fffff, r0
  3f:	77 0f ff ff 7f                	cmp	#0x7fffff, r15
  44:	74 00 00 00 00 80             	cmp	#0x80000000, r0
  4a:	74 0f 00 00 00 80             	cmp	#0x80000000, r15
  50:	74 00 ff ff ff 7f             	cmp	#0x7fffffff, r0
  56:	74 0f ff ff ff 7f             	cmp	#0x7fffffff, r15
  5c:	47 00                         	cmp	r0, r0
  5e:	47 0f                         	cmp	r0, r15
  60:	47 f0                         	cmp	r15, r0
  62:	47 ff                         	cmp	r15, r15
  64:	44 00                         	cmp	\[r0\]\.ub, r0
  66:	44 0f                         	cmp	\[r0\]\.ub, r15
  68:	06 04 00                      	cmp	\[r0\]\.b, r0
  6b:	06 04 0f                      	cmp	\[r0\]\.b, r15
  6e:	06 c4 00                      	cmp	\[r0\]\.uw, r0
  71:	06 c4 0f                      	cmp	\[r0\]\.uw, r15
  74:	06 44 00                      	cmp	\[r0\]\.w, r0
  77:	06 44 0f                      	cmp	\[r0\]\.w, r15
  7a:	06 84 00                      	cmp	\[r0\]\.l, r0
  7d:	06 84 0f                      	cmp	\[r0\]\.l, r15
  80:	44 f0                         	cmp	\[r15\]\.ub, r0
  82:	44 ff                         	cmp	\[r15\]\.ub, r15
  84:	06 04 f0                      	cmp	\[r15\]\.b, r0
  87:	06 04 ff                      	cmp	\[r15\]\.b, r15
  8a:	06 c4 f0                      	cmp	\[r15\]\.uw, r0
  8d:	06 c4 ff                      	cmp	\[r15\]\.uw, r15
  90:	06 44 f0                      	cmp	\[r15\]\.w, r0
  93:	06 44 ff                      	cmp	\[r15\]\.w, r15
  96:	06 84 f0                      	cmp	\[r15\]\.l, r0
  99:	06 84 ff                      	cmp	\[r15\]\.l, r15
  9c:	45 00 fc                      	cmp	252\[r0\]\.ub, r0
  9f:	45 0f fc                      	cmp	252\[r0\]\.ub, r15
  a2:	06 05 00 fc                   	cmp	252\[r0\]\.b, r0
  a6:	06 05 0f fc                   	cmp	252\[r0\]\.b, r15
  aa:	06 c5 00 7e                   	cmp	252\[r0\]\.uw, r0
  ae:	06 c5 0f 7e                   	cmp	252\[r0\]\.uw, r15
  b2:	06 45 00 7e                   	cmp	252\[r0\]\.w, r0
  b6:	06 45 0f 7e                   	cmp	252\[r0\]\.w, r15
  ba:	06 85 00 3f                   	cmp	252\[r0\]\.l, r0
  be:	06 85 0f 3f                   	cmp	252\[r0\]\.l, r15
  c2:	45 f0 fc                      	cmp	252\[r15\]\.ub, r0
  c5:	45 ff fc                      	cmp	252\[r15\]\.ub, r15
  c8:	06 05 f0 fc                   	cmp	252\[r15\]\.b, r0
  cc:	06 05 ff fc                   	cmp	252\[r15\]\.b, r15
  d0:	06 c5 f0 7e                   	cmp	252\[r15\]\.uw, r0
  d4:	06 c5 ff 7e                   	cmp	252\[r15\]\.uw, r15
  d8:	06 45 f0 7e                   	cmp	252\[r15\]\.w, r0
  dc:	06 45 ff 7e                   	cmp	252\[r15\]\.w, r15
  e0:	06 85 f0 3f                   	cmp	252\[r15\]\.l, r0
  e4:	06 85 ff 3f                   	cmp	252\[r15\]\.l, r15
  e8:	46 00 fc ff                   	cmp	65532\[r0\]\.ub, r0
  ec:	46 0f fc ff                   	cmp	65532\[r0\]\.ub, r15
  f0:	06 06 00 fc ff                	cmp	65532\[r0\]\.b, r0
  f5:	06 06 0f fc ff                	cmp	65532\[r0\]\.b, r15
  fa:	06 c6 00 fe 7f                	cmp	65532\[r0\]\.uw, r0
  ff:	06 c6 0f fe 7f                	cmp	65532\[r0\]\.uw, r15
 104:	06 46 00 fe 7f                	cmp	65532\[r0\]\.w, r0
 109:	06 46 0f fe 7f                	cmp	65532\[r0\]\.w, r15
 10e:	06 86 00 ff 3f                	cmp	65532\[r0\]\.l, r0
 113:	06 86 0f ff 3f                	cmp	65532\[r0\]\.l, r15
 118:	46 f0 fc ff                   	cmp	65532\[r15\]\.ub, r0
 11c:	46 ff fc ff                   	cmp	65532\[r15\]\.ub, r15
 120:	06 06 f0 fc ff                	cmp	65532\[r15\]\.b, r0
 125:	06 06 ff fc ff                	cmp	65532\[r15\]\.b, r15
 12a:	06 c6 f0 fe 7f                	cmp	65532\[r15\]\.uw, r0
 12f:	06 c6 ff fe 7f                	cmp	65532\[r15\]\.uw, r15
 134:	06 46 f0 fe 7f                	cmp	65532\[r15\]\.w, r0
 139:	06 46 ff fe 7f                	cmp	65532\[r15\]\.w, r15
 13e:	06 86 f0 ff 3f                	cmp	65532\[r15\]\.l, r0
 143:	06 86 ff ff 3f                	cmp	65532\[r15\]\.l, r15
