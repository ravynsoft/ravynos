#as: -mppc -me6500
#objdump: -dr -Me6500
#name: Power E6500 tests

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(10 01 14 03|03 14 01 10) 	vabsdub v0,v1,v2
   4:	(10 01 14 43|43 14 01 10) 	vabsduh v0,v1,v2
   8:	(10 01 14 83|83 14 01 10) 	vabsduw v0,v1,v2
   c:	(7c 01 10 dc|dc 10 01 7c) 	mvidsplt v0,r1,r2
  10:	(7c 01 10 5c|5c 10 01 7c) 	mviwsplt v0,r1,r2
  14:	(7c 00 12 0a|0a 12 00 7c) 	lvexbx  v0,0,r2
  18:	(7c 01 12 0a|0a 12 01 7c) 	lvexbx  v0,r1,r2
  1c:	(7c 00 12 4a|4a 12 00 7c) 	lvexhx  v0,0,r2
  20:	(7c 01 12 4a|4a 12 01 7c) 	lvexhx  v0,r1,r2
  24:	(7c 00 12 8a|8a 12 00 7c) 	lvexwx  v0,0,r2
  28:	(7c 01 12 8a|8a 12 01 7c) 	lvexwx  v0,r1,r2
  2c:	(7c 00 13 0a|0a 13 00 7c) 	stvexbx v0,0,r2
  30:	(7c 01 13 0a|0a 13 01 7c) 	stvexbx v0,r1,r2
  34:	(7c 00 13 4a|4a 13 00 7c) 	stvexhx v0,0,r2
  38:	(7c 01 13 4a|4a 13 01 7c) 	stvexhx v0,r1,r2
  3c:	(7c 00 13 8a|8a 13 00 7c) 	stvexwx v0,0,r2
  40:	(7c 01 13 8a|8a 13 01 7c) 	stvexwx v0,r1,r2
  44:	(7c 00 12 4e|4e 12 00 7c) 	lvepx   v0,0,r2
  48:	(7c 01 12 4e|4e 12 01 7c) 	lvepx   v0,r1,r2
  4c:	(7c 00 12 0e|0e 12 00 7c) 	lvepxl  v0,0,r2
  50:	(7c 01 12 0e|0e 12 01 7c) 	lvepxl  v0,r1,r2
  54:	(7c 00 16 4e|4e 16 00 7c) 	stvepx  v0,0,r2
  58:	(7c 01 16 4e|4e 16 01 7c) 	stvepx  v0,r1,r2
  5c:	(7c 00 16 0e|0e 16 00 7c) 	stvepxl v0,0,r2
  60:	(7c 01 16 0e|0e 16 01 7c) 	stvepxl v0,r1,r2
  64:	(7c 00 14 8a|8a 14 00 7c) 	lvtlx   v0,0,r2
  68:	(7c 01 14 8a|8a 14 01 7c) 	lvtlx   v0,r1,r2
  6c:	(7c 00 16 8a|8a 16 00 7c) 	lvtlxl  v0,0,r2
  70:	(7c 01 16 8a|8a 16 01 7c) 	lvtlxl  v0,r1,r2
  74:	(7c 00 14 4a|4a 14 00 7c) 	lvtrx   v0,0,r2
  78:	(7c 01 14 4a|4a 14 01 7c) 	lvtrx   v0,r1,r2
  7c:	(7c 00 16 4a|4a 16 00 7c) 	lvtrxl  v0,0,r2
  80:	(7c 01 16 4a|4a 16 01 7c) 	lvtrxl  v0,r1,r2
  84:	(7c 00 15 8a|8a 15 00 7c) 	stvflx  v0,0,r2
  88:	(7c 01 15 8a|8a 15 01 7c) 	stvflx  v0,r1,r2
  8c:	(7c 00 17 8a|8a 17 00 7c) 	stvflxl v0,0,r2
  90:	(7c 01 17 8a|8a 17 01 7c) 	stvflxl v0,r1,r2
  94:	(7c 00 15 4a|4a 15 00 7c) 	stvfrx  v0,0,r2
  98:	(7c 01 15 4a|4a 15 01 7c) 	stvfrx  v0,r1,r2
  9c:	(7c 00 17 4a|4a 17 00 7c) 	stvfrxl v0,0,r2
  a0:	(7c 01 17 4a|4a 17 01 7c) 	stvfrxl v0,r1,r2
  a4:	(7c 00 14 ca|ca 14 00 7c) 	lvswx   v0,0,r2
  a8:	(7c 01 14 ca|ca 14 01 7c) 	lvswx   v0,r1,r2
  ac:	(7c 00 16 ca|ca 16 00 7c) 	lvswxl  v0,0,r2
  b0:	(7c 01 16 ca|ca 16 01 7c) 	lvswxl  v0,r1,r2
  b4:	(7c 00 15 ca|ca 15 00 7c) 	stvswx  v0,0,r2
  b8:	(7c 01 15 ca|ca 15 01 7c) 	stvswx  v0,r1,r2
  bc:	(7c 00 17 ca|ca 17 00 7c) 	stvswxl v0,0,r2
  c0:	(7c 01 17 ca|ca 17 01 7c) 	stvswxl v0,r1,r2
  c4:	(7c 00 16 0a|0a 16 00 7c) 	lvsm    v0,0,r2
  c8:	(7c 01 16 0a|0a 16 01 7c) 	lvsm    v0,r1,r2
  cc:	(7f 5a d3 78|78 d3 5a 7f) 	miso
  d0:	(7c 00 04 ac|ac 04 00 7c) 	sync
  d4:	(7c 00 04 ac|ac 04 00 7c) 	sync
  d8:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
  dc:	(7c 21 04 ac|ac 04 21 7c) 	sync    1,1
  e0:	(7c 07 04 ac|ac 04 07 7c) 	sync    0,7
  e4:	(7c 28 04 ac|ac 04 28 7c) 	sync    1,8
  e8:	(7c 00 00 c3|c3 00 00 7c) 	dni     0,0
  ec:	(7f ff 00 c3|c3 00 ff 7f) 	dni     31,31
  f0:	(7c 40 0b 4d|4d 0b 40 7c) 	dcblq.  2,0,r1
  f4:	(7c 43 0b 4d|4d 0b 43 7c) 	dcblq.  2,r3,r1
  f8:	(7c 40 09 8d|8d 09 40 7c) 	icblq.  2,0,r1
  fc:	(7c 43 09 8d|8d 09 43 7c) 	icblq.  2,r3,r1
 100:	(7c 10 02 dc|dc 02 10 7c) 	mftmr   r0,16
 104:	(7c 10 03 dc|dc 03 10 7c) 	mttmr   16,r0
.*:	(7e 80 38 68|68 38 80 7e) 	lbarx   r20,0,r7
.*:	(7e 81 38 68|68 38 81 7e) 	lbarx   r20,r1,r7
.*:	(7e a0 40 e8|e8 40 a0 7e) 	lharx   r21,0,r8
.*:	(7e a1 40 e8|e8 40 a1 7e) 	lharx   r21,r1,r8
.*:	(7e c0 48 28|28 48 c0 7e) 	lwarx   r22,0,r9
.*:	(7e c1 48 28|28 48 c1 7e) 	lwarx   r22,r1,r9
.*:	(7e e0 50 a8|a8 50 e0 7e) 	ldarx   r23,0,r10
.*:	(7e e1 50 a8|a8 50 e1 7e) 	ldarx   r23,r1,r10
.*:	(7d 40 3d 6d|6d 3d 40 7d) 	stbcx\.  r10,0,r7
.*:	(7d 41 3d 6d|6d 3d 41 7d) 	stbcx\.  r10,r1,r7
.*:	(7d 60 45 ad|ad 45 60 7d) 	sthcx\.  r11,0,r8
.*:	(7d 61 45 ad|ad 45 61 7d) 	sthcx\.  r11,r1,r8
.*:	(7d 80 49 2d|2d 49 80 7d) 	stwcx\.  r12,0,r9
.*:	(7d 81 49 2d|2d 49 81 7d) 	stwcx\.  r12,r1,r9
.*:	(7d a0 51 ad|ad 51 a0 7d) 	stdcx\.  r13,0,r10
.*:	(7d a1 51 ad|ad 51 a1 7d) 	stdcx\.  r13,r1,r10
#pass
