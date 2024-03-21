#source: ./and.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	64 00                         	and	#0, r0
   2:	64 0f                         	and	#0, r15
   4:	64 f0                         	and	#15, r0
   6:	64 ff                         	and	#15, r15
   8:	75 20 80                      	and	#-128, r0
   b:	75 2f 80                      	and	#-128, r15
   e:	75 20 7f                      	and	#127, r0
  11:	75 2f 7f                      	and	#127, r15
  14:	76 20 00 80                   	and	#0xffff8000, r0
  18:	76 2f 00 80                   	and	#0xffff8000, r15
  1c:	77 20 00 80 00                	and	#0x8000, r0
  21:	77 2f 00 80 00                	and	#0x8000, r15
  26:	77 20 00 00 80                	and	#0xff800000, r0
  2b:	77 2f 00 00 80                	and	#0xff800000, r15
  30:	77 20 ff ff 7f                	and	#0x7fffff, r0
  35:	77 2f ff ff 7f                	and	#0x7fffff, r15
  3a:	74 20 00 00 00 80             	and	#0x80000000, r0
  40:	74 2f 00 00 00 80             	and	#0x80000000, r15
  46:	74 20 ff ff ff 7f             	and	#0x7fffffff, r0
  4c:	74 2f ff ff ff 7f             	and	#0x7fffffff, r15
  52:	53 00                         	and	r0, r0
  54:	53 0f                         	and	r0, r15
  56:	53 f0                         	and	r15, r0
  58:	53 ff                         	and	r15, r15
  5a:	50 00                         	and	\[r0\]\.ub, r0
  5c:	50 0f                         	and	\[r0\]\.ub, r15
  5e:	06 10 00                      	and	\[r0\]\.b, r0
  61:	06 10 0f                      	and	\[r0\]\.b, r15
  64:	06 d0 00                      	and	\[r0\]\.uw, r0
  67:	06 d0 0f                      	and	\[r0\]\.uw, r15
  6a:	06 50 00                      	and	\[r0\]\.w, r0
  6d:	06 50 0f                      	and	\[r0\]\.w, r15
  70:	06 90 00                      	and	\[r0\]\.l, r0
  73:	06 90 0f                      	and	\[r0\]\.l, r15
  76:	50 f0                         	and	\[r15\]\.ub, r0
  78:	50 ff                         	and	\[r15\]\.ub, r15
  7a:	06 10 f0                      	and	\[r15\]\.b, r0
  7d:	06 10 ff                      	and	\[r15\]\.b, r15
  80:	06 d0 f0                      	and	\[r15\]\.uw, r0
  83:	06 d0 ff                      	and	\[r15\]\.uw, r15
  86:	06 50 f0                      	and	\[r15\]\.w, r0
  89:	06 50 ff                      	and	\[r15\]\.w, r15
  8c:	06 90 f0                      	and	\[r15\]\.l, r0
  8f:	06 90 ff                      	and	\[r15\]\.l, r15
  92:	51 00 fc                      	and	252\[r0\]\.ub, r0
  95:	51 0f fc                      	and	252\[r0\]\.ub, r15
  98:	06 11 00 fc                   	and	252\[r0\]\.b, r0
  9c:	06 11 0f fc                   	and	252\[r0\]\.b, r15
  a0:	06 d1 00 7e                   	and	252\[r0\]\.uw, r0
  a4:	06 d1 0f 7e                   	and	252\[r0\]\.uw, r15
  a8:	06 51 00 7e                   	and	252\[r0\]\.w, r0
  ac:	06 51 0f 7e                   	and	252\[r0\]\.w, r15
  b0:	06 91 00 3f                   	and	252\[r0\]\.l, r0
  b4:	06 91 0f 3f                   	and	252\[r0\]\.l, r15
  b8:	51 f0 fc                      	and	252\[r15\]\.ub, r0
  bb:	51 ff fc                      	and	252\[r15\]\.ub, r15
  be:	06 11 f0 fc                   	and	252\[r15\]\.b, r0
  c2:	06 11 ff fc                   	and	252\[r15\]\.b, r15
  c6:	06 d1 f0 7e                   	and	252\[r15\]\.uw, r0
  ca:	06 d1 ff 7e                   	and	252\[r15\]\.uw, r15
  ce:	06 51 f0 7e                   	and	252\[r15\]\.w, r0
  d2:	06 51 ff 7e                   	and	252\[r15\]\.w, r15
  d6:	06 91 f0 3f                   	and	252\[r15\]\.l, r0
  da:	06 91 ff 3f                   	and	252\[r15\]\.l, r15
  de:	52 00 fc ff                   	and	65532\[r0\]\.ub, r0
  e2:	52 0f fc ff                   	and	65532\[r0\]\.ub, r15
  e6:	06 12 00 fc ff                	and	65532\[r0\]\.b, r0
  eb:	06 12 0f fc ff                	and	65532\[r0\]\.b, r15
  f0:	06 d2 00 fe 7f                	and	65532\[r0\]\.uw, r0
  f5:	06 d2 0f fe 7f                	and	65532\[r0\]\.uw, r15
  fa:	06 52 00 fe 7f                	and	65532\[r0\]\.w, r0
  ff:	06 52 0f fe 7f                	and	65532\[r0\]\.w, r15
 104:	06 92 00 ff 3f                	and	65532\[r0\]\.l, r0
 109:	06 92 0f ff 3f                	and	65532\[r0\]\.l, r15
 10e:	52 f0 fc ff                   	and	65532\[r15\]\.ub, r0
 112:	52 ff fc ff                   	and	65532\[r15\]\.ub, r15
 116:	06 12 f0 fc ff                	and	65532\[r15\]\.b, r0
 11b:	06 12 ff fc ff                	and	65532\[r15\]\.b, r15
 120:	06 d2 f0 fe 7f                	and	65532\[r15\]\.uw, r0
 125:	06 d2 ff fe 7f                	and	65532\[r15\]\.uw, r15
 12a:	06 52 f0 fe 7f                	and	65532\[r15\]\.w, r0
 12f:	06 52 ff fe 7f                	and	65532\[r15\]\.w, r15
 134:	06 92 f0 ff 3f                	and	65532\[r15\]\.l, r0
 139:	06 92 ff ff 3f                	and	65532\[r15\]\.l, r15
 13e:	ff 40 00                      	and	r0, r0, r0
 141:	ff 4f 00                      	and	r0, r0, r15
 144:	ff 40 0f                      	and	r0, r15, r0
 147:	ff 4f 0f                      	and	r0, r15, r15
 14a:	ff 40 f0                      	and	r15, r0, r0
 14d:	ff 4f f0                      	and	r15, r0, r15
 150:	ff 40 ff                      	and	r15, r15, r0
 153:	ff 4f ff                      	and	r15, r15, r15
