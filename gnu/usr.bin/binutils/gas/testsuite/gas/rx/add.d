#source: ./add.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	62 00                         	add	#0, r0
   2:	62 0f                         	add	#0, r15
   4:	62 f0                         	add	#15, r0
   6:	62 ff                         	add	#15, r15
   8:	71 00 80                      	add	#-128, r0, r0
   b:	71 ff 80                      	add	#-128, r15, r15
   e:	71 00 7f                      	add	#127, r0, r0
  11:	71 ff 7f                      	add	#127, r15, r15
  14:	72 00 00 80                   	add	#0xffff8000, r0, r0
  18:	72 ff 00 80                   	add	#0xffff8000, r15, r15
  1c:	73 00 00 80 00                	add	#0x8000, r0, r0
  21:	73 ff 00 80 00                	add	#0x8000, r15, r15
  26:	73 00 00 00 80                	add	#0xff800000, r0, r0
  2b:	73 ff 00 00 80                	add	#0xff800000, r15, r15
  30:	73 00 ff ff 7f                	add	#0x7fffff, r0, r0
  35:	73 ff ff ff 7f                	add	#0x7fffff, r15, r15
  3a:	70 00 00 00 00 80             	add	#0x80000000, r0, r0
  40:	70 ff 00 00 00 80             	add	#0x80000000, r15, r15
  46:	70 00 ff ff ff 7f             	add	#0x7fffffff, r0, r0
  4c:	70 ff ff ff ff 7f             	add	#0x7fffffff, r15, r15
  52:	4b 00                         	add	r0, r0
  54:	4b 0f                         	add	r0, r15
  56:	4b f0                         	add	r15, r0
  58:	4b ff                         	add	r15, r15
  5a:	48 00                         	add	\[r0\]\.ub, r0
  5c:	48 0f                         	add	\[r0\]\.ub, r15
  5e:	06 08 00                      	add	\[r0\]\.b, r0
  61:	06 08 0f                      	add	\[r0\]\.b, r15
  64:	06 c8 00                      	add	\[r0\]\.uw, r0
  67:	06 c8 0f                      	add	\[r0\]\.uw, r15
  6a:	06 48 00                      	add	\[r0\]\.w, r0
  6d:	06 48 0f                      	add	\[r0\]\.w, r15
  70:	06 88 00                      	add	\[r0\]\.l, r0
  73:	06 88 0f                      	add	\[r0\]\.l, r15
  76:	48 f0                         	add	\[r15\]\.ub, r0
  78:	48 ff                         	add	\[r15\]\.ub, r15
  7a:	06 08 f0                      	add	\[r15\]\.b, r0
  7d:	06 08 ff                      	add	\[r15\]\.b, r15
  80:	06 c8 f0                      	add	\[r15\]\.uw, r0
  83:	06 c8 ff                      	add	\[r15\]\.uw, r15
  86:	06 48 f0                      	add	\[r15\]\.w, r0
  89:	06 48 ff                      	add	\[r15\]\.w, r15
  8c:	06 88 f0                      	add	\[r15\]\.l, r0
  8f:	06 88 ff                      	add	\[r15\]\.l, r15
  92:	49 00 fc                      	add	252\[r0\]\.ub, r0
  95:	49 0f fc                      	add	252\[r0\]\.ub, r15
  98:	06 09 00 fc                   	add	252\[r0\]\.b, r0
  9c:	06 09 0f fc                   	add	252\[r0\]\.b, r15
  a0:	06 c9 00 7e                   	add	252\[r0\]\.uw, r0
  a4:	06 c9 0f 7e                   	add	252\[r0\]\.uw, r15
  a8:	06 49 00 7e                   	add	252\[r0\]\.w, r0
  ac:	06 49 0f 7e                   	add	252\[r0\]\.w, r15
  b0:	06 89 00 3f                   	add	252\[r0\]\.l, r0
  b4:	06 89 0f 3f                   	add	252\[r0\]\.l, r15
  b8:	49 f0 fc                      	add	252\[r15\]\.ub, r0
  bb:	49 ff fc                      	add	252\[r15\]\.ub, r15
  be:	06 09 f0 fc                   	add	252\[r15\]\.b, r0
  c2:	06 09 ff fc                   	add	252\[r15\]\.b, r15
  c6:	06 c9 f0 7e                   	add	252\[r15\]\.uw, r0
  ca:	06 c9 ff 7e                   	add	252\[r15\]\.uw, r15
  ce:	06 49 f0 7e                   	add	252\[r15\]\.w, r0
  d2:	06 49 ff 7e                   	add	252\[r15\]\.w, r15
  d6:	06 89 f0 3f                   	add	252\[r15\]\.l, r0
  da:	06 89 ff 3f                   	add	252\[r15\]\.l, r15
  de:	4a 00 fc ff                   	add	65532\[r0\]\.ub, r0
  e2:	4a 0f fc ff                   	add	65532\[r0\]\.ub, r15
  e6:	06 0a 00 fc ff                	add	65532\[r0\]\.b, r0
  eb:	06 0a 0f fc ff                	add	65532\[r0\]\.b, r15
  f0:	06 ca 00 fe 7f                	add	65532\[r0\]\.uw, r0
  f5:	06 ca 0f fe 7f                	add	65532\[r0\]\.uw, r15
  fa:	06 4a 00 fe 7f                	add	65532\[r0\]\.w, r0
  ff:	06 4a 0f fe 7f                	add	65532\[r0\]\.w, r15
 104:	06 8a 00 ff 3f                	add	65532\[r0\]\.l, r0
 109:	06 8a 0f ff 3f                	add	65532\[r0\]\.l, r15
 10e:	4a f0 fc ff                   	add	65532\[r15\]\.ub, r0
 112:	4a ff fc ff                   	add	65532\[r15\]\.ub, r15
 116:	06 0a f0 fc ff                	add	65532\[r15\]\.b, r0
 11b:	06 0a ff fc ff                	add	65532\[r15\]\.b, r15
 120:	06 ca f0 fe 7f                	add	65532\[r15\]\.uw, r0
 125:	06 ca ff fe 7f                	add	65532\[r15\]\.uw, r15
 12a:	06 4a f0 fe 7f                	add	65532\[r15\]\.w, r0
 12f:	06 4a ff fe 7f                	add	65532\[r15\]\.w, r15
 134:	06 8a f0 ff 3f                	add	65532\[r15\]\.l, r0
 139:	06 8a ff ff 3f                	add	65532\[r15\]\.l, r15
 13e:	71 00 80                      	add	#-128, r0, r0
 141:	71 0f 80                      	add	#-128, r0, r15
 144:	71 f0 80                      	add	#-128, r15, r0
 147:	71 ff 80                      	add	#-128, r15, r15
 14a:	71 00 7f                      	add	#127, r0, r0
 14d:	71 0f 7f                      	add	#127, r0, r15
 150:	71 f0 7f                      	add	#127, r15, r0
 153:	71 ff 7f                      	add	#127, r15, r15
 156:	72 00 00 80                   	add	#0xffff8000, r0, r0
 15a:	72 0f 00 80                   	add	#0xffff8000, r0, r15
 15e:	72 f0 00 80                   	add	#0xffff8000, r15, r0
 162:	72 ff 00 80                   	add	#0xffff8000, r15, r15
 166:	73 00 00 80 00                	add	#0x8000, r0, r0
 16b:	73 0f 00 80 00                	add	#0x8000, r0, r15
 170:	73 f0 00 80 00                	add	#0x8000, r15, r0
 175:	73 ff 00 80 00                	add	#0x8000, r15, r15
 17a:	73 00 00 00 80                	add	#0xff800000, r0, r0
 17f:	73 0f 00 00 80                	add	#0xff800000, r0, r15
 184:	73 f0 00 00 80                	add	#0xff800000, r15, r0
 189:	73 ff 00 00 80                	add	#0xff800000, r15, r15
 18e:	73 00 ff ff 7f                	add	#0x7fffff, r0, r0
 193:	73 0f ff ff 7f                	add	#0x7fffff, r0, r15
 198:	73 f0 ff ff 7f                	add	#0x7fffff, r15, r0
 19d:	73 ff ff ff 7f                	add	#0x7fffff, r15, r15
 1a2:	70 00 00 00 00 80             	add	#0x80000000, r0, r0
 1a8:	70 0f 00 00 00 80             	add	#0x80000000, r0, r15
 1ae:	70 f0 00 00 00 80             	add	#0x80000000, r15, r0
 1b4:	70 ff 00 00 00 80             	add	#0x80000000, r15, r15
 1ba:	70 00 ff ff ff 7f             	add	#0x7fffffff, r0, r0
 1c0:	70 0f ff ff ff 7f             	add	#0x7fffffff, r0, r15
 1c6:	70 f0 ff ff ff 7f             	add	#0x7fffffff, r15, r0
 1cc:	70 ff ff ff ff 7f             	add	#0x7fffffff, r15, r15
 1d2:	ff 20 00                      	add	r0, r0, r0
 1d5:	ff 2f 00                      	add	r0, r0, r15
 1d8:	ff 20 0f                      	add	r0, r15, r0
 1db:	ff 2f 0f                      	add	r0, r15, r15
 1de:	ff 20 f0                      	add	r15, r0, r0
 1e1:	ff 2f f0                      	add	r15, r0, r15
 1e4:	ff 20 ff                      	add	r15, r15, r0
 1e7:	ff 2f ff                      	add	r15, r15, r15
