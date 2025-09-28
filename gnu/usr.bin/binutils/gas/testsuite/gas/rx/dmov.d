#source: ./dmov.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 77 80 03                   	dmov\.d	r0, drh0
   4:	fd 77 80 f3                   	dmov\.d	r0, drh15
   8:	fd 77 8f 03                   	dmov\.d	r15, drh0
   c:	fd 77 8f f3                   	dmov\.d	r15, drh15
  10:	fd 77 80 02                   	dmov\.l	r0, drh0
  14:	fd 77 80 f2                   	dmov\.l	r0, drh15
  18:	fd 77 8f 02                   	dmov\.l	r15, drh0
  1c:	fd 77 8f f2                   	dmov\.l	r15, drh15
  20:	fd 77 80 00                   	dmov\.l	r0, drl0
  24:	fd 77 80 f0                   	dmov\.l	r0, drl15
  28:	fd 77 8f 00                   	dmov\.l	r15, drl0
  2c:	fd 77 8f f0                   	dmov\.l	r15, drl15
  30:	fd 75 80 02                   	dmov\.l	drh0, r0
  34:	fd 75 8f 02                   	dmov\.l	drh0, r15
  38:	fd 75 80 f2                   	dmov\.l	drh15, r0
  3c:	fd 75 8f f2                   	dmov\.l	drh15, r15
  40:	fd 75 80 00                   	dmov\.l	drl0, r0
  44:	fd 75 8f 00                   	dmov\.l	drl0, r15
  48:	fd 75 80 f0                   	dmov\.l	drl15, r0
  4c:	fd 75 8f f0                   	dmov\.l	drl15, r15
  50:	76 90 0c 00                   	dmov\.d	dr0, dr0
  54:	76 90 0c f0                   	dmov\.d	dr0, dr15
  58:	76 90 fc 00                   	dmov\.d	dr15, dr0
  5c:	76 90 fc f0                   	dmov\.d	dr15, dr15
  60:	fc 78 08 00                   	dmov\.d	dr0, \[r0\]
  64:	fc 78 f8 00                   	dmov\.d	dr0, \[r15\]
  68:	fc 78 08 f0                   	dmov\.d	dr15, \[r0\]
  6c:	fc 78 f8 f0                   	dmov\.d	dr15, \[r15\]
  70:	fc 78 08 00                   	dmov\.d	dr0, \[r0\]
  74:	fc 78 f8 00                   	dmov\.d	dr0, \[r15\]
  78:	fc 7a 08 ff 01 00             	dmov\.d	dr0, 4088\[r0\]
  7e:	fc 7a f8 ff 01 00             	dmov\.d	dr0, 4088\[r15\]
  84:	fc 7a 08 fc ff 00             	dmov\.d	dr0, 524256\[r0\]
  8a:	fc 7a f8 fc ff 00             	dmov\.d	dr0, 524256\[r15\]
  90:	fc 78 08 f0                   	dmov\.d	dr15, \[r0\]
  94:	fc 78 f8 f0                   	dmov\.d	dr15, \[r15\]
  98:	fc 7a 08 ff 01 f0             	dmov\.d	dr15, 4088\[r0\]
  9e:	fc 7a f8 ff 01 f0             	dmov\.d	dr15, 4088\[r15\]
  a4:	fc 7a 08 fc ff f0             	dmov\.d	dr15, 524256\[r0\]
  aa:	fc 7a f8 fc ff f0             	dmov\.d	dr15, 524256\[r15\]
  b0:	fc c8 08 00                   	dmov\.d	\[r0\], dr0
  b4:	fc c8 08 f0                   	dmov\.d	\[r0\], dr15
  b8:	fc c8 f8 00                   	dmov\.d	\[r15\], dr0
  bc:	fc c8 f8 f0                   	dmov\.d	\[r15\], dr15
  c0:	fc c8 08 00                   	dmov\.d	\[r0\], dr0
  c4:	fc c8 08 f0                   	dmov\.d	\[r0\], dr15
  c8:	fc c8 f8 00                   	dmov\.d	\[r15\], dr0
  cc:	fc c8 f8 f0                   	dmov\.d	\[r15\], dr15
  d0:	fc ca 08 ff 01 00             	dmov\.d	4088\[r0\], dr0
  d6:	fc ca 08 ff 01 f0             	dmov\.d	4088\[r0\], dr15
  dc:	fc ca f8 ff 01 00             	dmov\.d	4088\[r15\], dr0
  e2:	fc ca f8 ff 01 f0             	dmov\.d	4088\[r15\], dr15
  e8:	fc ca 08 fc ff 00             	dmov\.d	524256\[r0\], dr0
  ee:	fc ca 08 fc ff f0             	dmov\.d	524256\[r0\], dr15
  f4:	fc ca f8 fc ff 00             	dmov\.d	524256\[r15\], dr0
  fa:	fc ca f8 fc ff f0             	dmov\.d	524256\[r15\], dr15
 100:	f9 03 03 00 00 00 80          	dmov\.d	#0x80000000, drh0
 107:	f9 03 f3 00 00 00 80          	dmov\.d	#0x80000000, drh15
 10e:	f9 03 03 ff ff ff ff          	dmov\.d	#-1, drh0
 115:	f9 03 f3 ff ff ff ff          	dmov\.d	#-1, drh15
 11c:	f9 03 02 00 00 00 80          	dmov\.l	#0x80000000, drh0
 123:	f9 03 f2 00 00 00 80          	dmov\.l	#0x80000000, drh15
 12a:	f9 03 02 ff ff ff ff          	dmov\.l	#-1, drh0
 131:	f9 03 f2 ff ff ff ff          	dmov\.l	#-1, drh15
 138:	f9 03 00 00 00 00 80          	dmov\.l	#0x80000000, drl0
 13f:	f9 03 f0 00 00 00 80          	dmov\.l	#0x80000000, drl15
 146:	f9 03 00 ff ff ff ff          	dmov\.l	#-1, drl0
 14d:	f9 03 f0 ff ff ff ff          	dmov\.l	#-1, drl15
