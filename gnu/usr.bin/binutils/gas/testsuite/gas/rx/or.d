#source: ./or.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	65 00                         	or	#0, r0
   2:	65 0f                         	or	#0, r15
   4:	65 f0                         	or	#15, r0
   6:	65 ff                         	or	#15, r15
   8:	75 30 80                      	or	#-128, r0
   b:	75 3f 80                      	or	#-128, r15
   e:	75 30 7f                      	or	#127, r0
  11:	75 3f 7f                      	or	#127, r15
  14:	76 30 00 80                   	or	#0xffff8000, r0
  18:	76 3f 00 80                   	or	#0xffff8000, r15
  1c:	77 30 00 80 00                	or	#0x8000, r0
  21:	77 3f 00 80 00                	or	#0x8000, r15
  26:	77 30 00 00 80                	or	#0xff800000, r0
  2b:	77 3f 00 00 80                	or	#0xff800000, r15
  30:	77 30 ff ff 7f                	or	#0x7fffff, r0
  35:	77 3f ff ff 7f                	or	#0x7fffff, r15
  3a:	74 30 00 00 00 80             	or	#0x80000000, r0
  40:	74 3f 00 00 00 80             	or	#0x80000000, r15
  46:	74 30 ff ff ff 7f             	or	#0x7fffffff, r0
  4c:	74 3f ff ff ff 7f             	or	#0x7fffffff, r15
  52:	57 00                         	or	r0, r0
  54:	57 0f                         	or	r0, r15
  56:	57 f0                         	or	r15, r0
  58:	57 ff                         	or	r15, r15
  5a:	54 00                         	or	\[r0\]\.ub, r0
  5c:	54 0f                         	or	\[r0\]\.ub, r15
  5e:	06 14 00                      	or	\[r0\]\.b, r0
  61:	06 14 0f                      	or	\[r0\]\.b, r15
  64:	06 d4 00                      	or	\[r0\]\.uw, r0
  67:	06 d4 0f                      	or	\[r0\]\.uw, r15
  6a:	06 54 00                      	or	\[r0\]\.w, r0
  6d:	06 54 0f                      	or	\[r0\]\.w, r15
  70:	06 94 00                      	or	\[r0\]\.l, r0
  73:	06 94 0f                      	or	\[r0\]\.l, r15
  76:	54 f0                         	or	\[r15\]\.ub, r0
  78:	54 ff                         	or	\[r15\]\.ub, r15
  7a:	06 14 f0                      	or	\[r15\]\.b, r0
  7d:	06 14 ff                      	or	\[r15\]\.b, r15
  80:	06 d4 f0                      	or	\[r15\]\.uw, r0
  83:	06 d4 ff                      	or	\[r15\]\.uw, r15
  86:	06 54 f0                      	or	\[r15\]\.w, r0
  89:	06 54 ff                      	or	\[r15\]\.w, r15
  8c:	06 94 f0                      	or	\[r15\]\.l, r0
  8f:	06 94 ff                      	or	\[r15\]\.l, r15
  92:	55 00 fc                      	or	252\[r0\]\.ub, r0
  95:	55 0f fc                      	or	252\[r0\]\.ub, r15
  98:	06 15 00 fc                   	or	252\[r0\]\.b, r0
  9c:	06 15 0f fc                   	or	252\[r0\]\.b, r15
  a0:	06 d5 00 7e                   	or	252\[r0\]\.uw, r0
  a4:	06 d5 0f 7e                   	or	252\[r0\]\.uw, r15
  a8:	06 55 00 7e                   	or	252\[r0\]\.w, r0
  ac:	06 55 0f 7e                   	or	252\[r0\]\.w, r15
  b0:	06 95 00 3f                   	or	252\[r0\]\.l, r0
  b4:	06 95 0f 3f                   	or	252\[r0\]\.l, r15
  b8:	55 f0 fc                      	or	252\[r15\]\.ub, r0
  bb:	55 ff fc                      	or	252\[r15\]\.ub, r15
  be:	06 15 f0 fc                   	or	252\[r15\]\.b, r0
  c2:	06 15 ff fc                   	or	252\[r15\]\.b, r15
  c6:	06 d5 f0 7e                   	or	252\[r15\]\.uw, r0
  ca:	06 d5 ff 7e                   	or	252\[r15\]\.uw, r15
  ce:	06 55 f0 7e                   	or	252\[r15\]\.w, r0
  d2:	06 55 ff 7e                   	or	252\[r15\]\.w, r15
  d6:	06 95 f0 3f                   	or	252\[r15\]\.l, r0
  da:	06 95 ff 3f                   	or	252\[r15\]\.l, r15
  de:	56 00 fc ff                   	or	65532\[r0\]\.ub, r0
  e2:	56 0f fc ff                   	or	65532\[r0\]\.ub, r15
  e6:	06 16 00 fc ff                	or	65532\[r0\]\.b, r0
  eb:	06 16 0f fc ff                	or	65532\[r0\]\.b, r15
  f0:	06 d6 00 fe 7f                	or	65532\[r0\]\.uw, r0
  f5:	06 d6 0f fe 7f                	or	65532\[r0\]\.uw, r15
  fa:	06 56 00 fe 7f                	or	65532\[r0\]\.w, r0
  ff:	06 56 0f fe 7f                	or	65532\[r0\]\.w, r15
 104:	06 96 00 ff 3f                	or	65532\[r0\]\.l, r0
 109:	06 96 0f ff 3f                	or	65532\[r0\]\.l, r15
 10e:	56 f0 fc ff                   	or	65532\[r15\]\.ub, r0
 112:	56 ff fc ff                   	or	65532\[r15\]\.ub, r15
 116:	06 16 f0 fc ff                	or	65532\[r15\]\.b, r0
 11b:	06 16 ff fc ff                	or	65532\[r15\]\.b, r15
 120:	06 d6 f0 fe 7f                	or	65532\[r15\]\.uw, r0
 125:	06 d6 ff fe 7f                	or	65532\[r15\]\.uw, r15
 12a:	06 56 f0 fe 7f                	or	65532\[r15\]\.w, r0
 12f:	06 56 ff fe 7f                	or	65532\[r15\]\.w, r15
 134:	06 96 f0 ff 3f                	or	65532\[r15\]\.l, r0
 139:	06 96 ff ff 3f                	or	65532\[r15\]\.l, r15
 13e:	ff 50 00                      	or	r0, r0, r0
 141:	ff 5f 00                      	or	r0, r0, r15
 144:	ff 50 0f                      	or	r0, r15, r0
 147:	ff 5f 0f                      	or	r0, r15, r15
 14a:	ff 50 f0                      	or	r15, r0, r0
 14d:	ff 5f f0                      	or	r15, r0, r15
 150:	ff 50 ff                      	or	r15, r15, r0
 153:	ff 5f ff                      	or	r15, r15, r15
