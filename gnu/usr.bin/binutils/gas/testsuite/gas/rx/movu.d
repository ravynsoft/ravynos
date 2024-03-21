#source: ./movu.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	b1 00                         	movu\.b	4\[r0\], r0
   2:	b1 07                         	movu\.b	4\[r0\], r7
   4:	b1 70                         	movu\.b	4\[r7\], r0
   6:	b1 77                         	movu\.b	4\[r7\], r7
   8:	b7 00                         	movu\.b	28\[r0\], r0
   a:	b7 07                         	movu\.b	28\[r0\], r7
   c:	b7 70                         	movu\.b	28\[r7\], r0
   e:	b7 77                         	movu\.b	28\[r7\], r7
  10:	b8 80                         	movu\.w	4\[r0\], r0
  12:	b8 87                         	movu\.w	4\[r0\], r7
  14:	b8 f0                         	movu\.w	4\[r7\], r0
  16:	b8 f7                         	movu\.w	4\[r7\], r7
  18:	bb 80                         	movu\.w	28\[r0\], r0
  1a:	bb 87                         	movu\.w	28\[r0\], r7
  1c:	bb f0                         	movu\.w	28\[r7\], r0
  1e:	bb f7                         	movu\.w	28\[r7\], r7
  20:	5b 00                         	movu\.b	r0, r0
  22:	5b 0f                         	movu\.b	r0, r15
  24:	5b f0                         	movu\.b	r15, r0
  26:	5b ff                         	movu\.b	r15, r15
  28:	5f 00                         	movu\.w	r0, r0
  2a:	5f 0f                         	movu\.w	r0, r15
  2c:	5f f0                         	movu\.w	r15, r0
  2e:	5f ff                         	movu\.w	r15, r15
  30:	58 00                         	movu\.b	\[r0\], r0
  32:	58 0f                         	movu\.b	\[r0\], r15
  34:	58 f0                         	movu\.b	\[r15\], r0
  36:	58 ff                         	movu\.b	\[r15\], r15
  38:	59 00 fc                      	movu\.b	252\[r0\], r0
  3b:	59 0f fc                      	movu\.b	252\[r0\], r15
  3e:	59 f0 fc                      	movu\.b	252\[r15\], r0
  41:	59 ff fc                      	movu\.b	252\[r15\], r15
  44:	5a 00 fc ff                   	movu\.b	65532\[r0\], r0
  48:	5a 0f fc ff                   	movu\.b	65532\[r0\], r15
  4c:	5a f0 fc ff                   	movu\.b	65532\[r15\], r0
  50:	5a ff fc ff                   	movu\.b	65532\[r15\], r15
  54:	5c 00                         	movu\.w	\[r0\], r0
  56:	5c 0f                         	movu\.w	\[r0\], r15
  58:	5c f0                         	movu\.w	\[r15\], r0
  5a:	5c ff                         	movu\.w	\[r15\], r15
  5c:	5d 00 7e                      	movu\.w	252\[r0\], r0
  5f:	5d 0f 7e                      	movu\.w	252\[r0\], r15
  62:	5d f0 7e                      	movu\.w	252\[r15\], r0
  65:	5d ff 7e                      	movu\.w	252\[r15\], r15
  68:	5e 00 fe 7f                   	movu\.w	65532\[r0\], r0
  6c:	5e 0f fe 7f                   	movu\.w	65532\[r0\], r15
  70:	5e f0 fe 7f                   	movu\.w	65532\[r15\], r0
  74:	5e ff fe 7f                   	movu\.w	65532\[r15\], r15
  78:	fe c0 00                      	movu\.b	\[r0, r0\], r0
  7b:	fe c0 0f                      	movu\.b	\[r0, r0\], r15
  7e:	fe c0 f0                      	movu\.b	\[r0, r15\], r0
  81:	fe c0 ff                      	movu\.b	\[r0, r15\], r15
  84:	fe cf 00                      	movu\.b	\[r15, r0\], r0
  87:	fe cf 0f                      	movu\.b	\[r15, r0\], r15
  8a:	fe cf f0                      	movu\.b	\[r15, r15\], r0
  8d:	fe cf ff                      	movu\.b	\[r15, r15\], r15
  90:	fe d0 00                      	movu\.w	\[r0, r0\], r0
  93:	fe d0 0f                      	movu\.w	\[r0, r0\], r15
  96:	fe d0 f0                      	movu\.w	\[r0, r15\], r0
  99:	fe d0 ff                      	movu\.w	\[r0, r15\], r15
  9c:	fe df 00                      	movu\.w	\[r15, r0\], r0
  9f:	fe df 0f                      	movu\.w	\[r15, r0\], r15
  a2:	fe df f0                      	movu\.w	\[r15, r15\], r0
  a5:	fe df ff                      	movu\.w	\[r15, r15\], r15
  a8:	fd 38 00                      	movu\.b	\[r0\+\], r0
  ab:	fd 38 0f                      	movu\.b	\[r0\+\], r15
  ae:	fd 38 f0                      	movu\.b	\[r15\+\], r0
  b1:	fd 38 ff                      	movu\.b	\[r15\+\], r15
  b4:	fd 39 00                      	movu\.w	\[r0\+\], r0
  b7:	fd 39 0f                      	movu\.w	\[r0\+\], r15
  ba:	fd 39 f0                      	movu\.w	\[r15\+\], r0
  bd:	fd 39 ff                      	movu\.w	\[r15\+\], r15
  c0:	fd 3c 00                      	movu\.b	\[-r0\], r0
  c3:	fd 3c 0f                      	movu\.b	\[-r0\], r15
  c6:	fd 3c f0                      	movu\.b	\[-r15\], r0
  c9:	fd 3c ff                      	movu\.b	\[-r15\], r15
  cc:	fd 3d 00                      	movu\.w	\[-r0\], r0
  cf:	fd 3d 0f                      	movu\.w	\[-r0\], r15
  d2:	fd 3d f0                      	movu\.w	\[-r15\], r0
  d5:	fd 3d ff                      	movu\.w	\[-r15\], r15
