#source: ./mul.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	63 00                         	mul	#0, r0
   2:	63 0f                         	mul	#0, r15
   4:	63 f0                         	mul	#15, r0
   6:	63 ff                         	mul	#15, r15
   8:	75 10 80                      	mul	#-128, r0
   b:	75 1f 80                      	mul	#-128, r15
   e:	75 10 7f                      	mul	#127, r0
  11:	75 1f 7f                      	mul	#127, r15
  14:	76 10 00 80                   	mul	#0xffff8000, r0
  18:	76 1f 00 80                   	mul	#0xffff8000, r15
  1c:	77 10 00 80 00                	mul	#0x8000, r0
  21:	77 1f 00 80 00                	mul	#0x8000, r15
  26:	77 10 00 00 80                	mul	#0xff800000, r0
  2b:	77 1f 00 00 80                	mul	#0xff800000, r15
  30:	77 10 ff ff 7f                	mul	#0x7fffff, r0
  35:	77 1f ff ff 7f                	mul	#0x7fffff, r15
  3a:	74 10 00 00 00 80             	mul	#0x80000000, r0
  40:	74 1f 00 00 00 80             	mul	#0x80000000, r15
  46:	74 10 ff ff ff 7f             	mul	#0x7fffffff, r0
  4c:	74 1f ff ff ff 7f             	mul	#0x7fffffff, r15
  52:	4f 00                         	mul	r0, r0
  54:	4f 0f                         	mul	r0, r15
  56:	4f f0                         	mul	r15, r0
  58:	4f ff                         	mul	r15, r15
  5a:	4c 00                         	mul	\[r0\]\.ub, r0
  5c:	4c 0f                         	mul	\[r0\]\.ub, r15
  5e:	06 0c 00                      	mul	\[r0\]\.b, r0
  61:	06 0c 0f                      	mul	\[r0\]\.b, r15
  64:	06 cc 00                      	mul	\[r0\]\.uw, r0
  67:	06 cc 0f                      	mul	\[r0\]\.uw, r15
  6a:	06 4c 00                      	mul	\[r0\]\.w, r0
  6d:	06 4c 0f                      	mul	\[r0\]\.w, r15
  70:	06 8c 00                      	mul	\[r0\]\.l, r0
  73:	06 8c 0f                      	mul	\[r0\]\.l, r15
  76:	4c f0                         	mul	\[r15\]\.ub, r0
  78:	4c ff                         	mul	\[r15\]\.ub, r15
  7a:	06 0c f0                      	mul	\[r15\]\.b, r0
  7d:	06 0c ff                      	mul	\[r15\]\.b, r15
  80:	06 cc f0                      	mul	\[r15\]\.uw, r0
  83:	06 cc ff                      	mul	\[r15\]\.uw, r15
  86:	06 4c f0                      	mul	\[r15\]\.w, r0
  89:	06 4c ff                      	mul	\[r15\]\.w, r15
  8c:	06 8c f0                      	mul	\[r15\]\.l, r0
  8f:	06 8c ff                      	mul	\[r15\]\.l, r15
  92:	4d 00 fc                      	mul	252\[r0\]\.ub, r0
  95:	4d 0f fc                      	mul	252\[r0\]\.ub, r15
  98:	06 0d 00 fc                   	mul	252\[r0\]\.b, r0
  9c:	06 0d 0f fc                   	mul	252\[r0\]\.b, r15
  a0:	06 cd 00 7e                   	mul	252\[r0\]\.uw, r0
  a4:	06 cd 0f 7e                   	mul	252\[r0\]\.uw, r15
  a8:	06 4d 00 7e                   	mul	252\[r0\]\.w, r0
  ac:	06 4d 0f 7e                   	mul	252\[r0\]\.w, r15
  b0:	06 8d 00 3f                   	mul	252\[r0\]\.l, r0
  b4:	06 8d 0f 3f                   	mul	252\[r0\]\.l, r15
  b8:	4d f0 fc                      	mul	252\[r15\]\.ub, r0
  bb:	4d ff fc                      	mul	252\[r15\]\.ub, r15
  be:	06 0d f0 fc                   	mul	252\[r15\]\.b, r0
  c2:	06 0d ff fc                   	mul	252\[r15\]\.b, r15
  c6:	06 cd f0 7e                   	mul	252\[r15\]\.uw, r0
  ca:	06 cd ff 7e                   	mul	252\[r15\]\.uw, r15
  ce:	06 4d f0 7e                   	mul	252\[r15\]\.w, r0
  d2:	06 4d ff 7e                   	mul	252\[r15\]\.w, r15
  d6:	06 8d f0 3f                   	mul	252\[r15\]\.l, r0
  da:	06 8d ff 3f                   	mul	252\[r15\]\.l, r15
  de:	4e 00 fc ff                   	mul	65532\[r0\]\.ub, r0
  e2:	4e 0f fc ff                   	mul	65532\[r0\]\.ub, r15
  e6:	06 0e 00 fc ff                	mul	65532\[r0\]\.b, r0
  eb:	06 0e 0f fc ff                	mul	65532\[r0\]\.b, r15
  f0:	06 ce 00 fe 7f                	mul	65532\[r0\]\.uw, r0
  f5:	06 ce 0f fe 7f                	mul	65532\[r0\]\.uw, r15
  fa:	06 4e 00 fe 7f                	mul	65532\[r0\]\.w, r0
  ff:	06 4e 0f fe 7f                	mul	65532\[r0\]\.w, r15
 104:	06 8e 00 ff 3f                	mul	65532\[r0\]\.l, r0
 109:	06 8e 0f ff 3f                	mul	65532\[r0\]\.l, r15
 10e:	4e f0 fc ff                   	mul	65532\[r15\]\.ub, r0
 112:	4e ff fc ff                   	mul	65532\[r15\]\.ub, r15
 116:	06 0e f0 fc ff                	mul	65532\[r15\]\.b, r0
 11b:	06 0e ff fc ff                	mul	65532\[r15\]\.b, r15
 120:	06 ce f0 fe 7f                	mul	65532\[r15\]\.uw, r0
 125:	06 ce ff fe 7f                	mul	65532\[r15\]\.uw, r15
 12a:	06 4e f0 fe 7f                	mul	65532\[r15\]\.w, r0
 12f:	06 4e ff fe 7f                	mul	65532\[r15\]\.w, r15
 134:	06 8e f0 ff 3f                	mul	65532\[r15\]\.l, r0
 139:	06 8e ff ff 3f                	mul	65532\[r15\]\.l, r15
 13e:	ff 30 00                      	mul 	r0, r0, r0
 141:	ff 3f 00                      	mul 	r0, r0, r15
 144:	ff 30 0f                      	mul 	r0, r15, r0
 147:	ff 3f 0f                      	mul 	r0, r15, r15
 14a:	ff 30 f0                      	mul 	r15, r0, r0
 14d:	ff 3f f0                      	mul 	r15, r0, r15
 150:	ff 30 ff                      	mul 	r15, r15, r0
 153:	ff 3f ff                      	mul 	r15, r15, r15
