#name: Valid v8-a+rdma
#as: -march=armv8-a+rdma
#objdump: -dr
#skip: *-*-pe *-*-wince
#source: armv8-a+rdma.s

.*: +file format .*arm.*


Disassembly of section .text:

00000000 <.*>:
   0:	f3110b12 	vqrdmlah.s16	d0, d1, d2
   4:	f3120b54 	vqrdmlah.s16	q0, q1, q2
   8:	f3210b12 	vqrdmlah.s32	d0, d1, d2
   c:	f3220b54 	vqrdmlah.s32	q0, q1, q2
  10:	f3110c12 	vqrdmlsh.s16	d0, d1, d2
  14:	f3120c54 	vqrdmlsh.s16	q0, q1, q2
  18:	f3210c12 	vqrdmlsh.s32	d0, d1, d2
  1c:	f3220c54 	vqrdmlsh.s32	q0, q1, q2
  20:	f2910e42 	vqrdmlah.s16	d0, d1, d2\[0\]
  24:	f2910e4a 	vqrdmlah.s16	d0, d1, d2\[1\]
  28:	f2910e62 	vqrdmlah.s16	d0, d1, d2\[2\]
  2c:	f2910e6a 	vqrdmlah.s16	d0, d1, d2\[3\]
  30:	f3920e42 	vqrdmlah.s16	q0, q1, d2\[0\]
  34:	f3920e4a 	vqrdmlah.s16	q0, q1, d2\[1\]
  38:	f3920e62 	vqrdmlah.s16	q0, q1, d2\[2\]
  3c:	f3920e6a 	vqrdmlah.s16	q0, q1, d2\[3\]
  40:	f2a10e42 	vqrdmlah.s32	d0, d1, d2\[0\]
  44:	f2a10e62 	vqrdmlah.s32	d0, d1, d2\[1\]
  48:	f3a20e42 	vqrdmlah.s32	q0, q1, d2\[0\]
  4c:	f3a20e62 	vqrdmlah.s32	q0, q1, d2\[1\]
  50:	f2910f42 	vqrdmlsh.s16	d0, d1, d2\[0\]
  54:	f2910f4a 	vqrdmlsh.s16	d0, d1, d2\[1\]
  58:	f2910f62 	vqrdmlsh.s16	d0, d1, d2\[2\]
  5c:	f2910f6a 	vqrdmlsh.s16	d0, d1, d2\[3\]
  60:	f3920f42 	vqrdmlsh.s16	q0, q1, d2\[0\]
  64:	f3920f4a 	vqrdmlsh.s16	q0, q1, d2\[1\]
  68:	f3920f62 	vqrdmlsh.s16	q0, q1, d2\[2\]
  6c:	f3920f6a 	vqrdmlsh.s16	q0, q1, d2\[3\]
  70:	f2a10f42 	vqrdmlsh.s32	d0, d1, d2\[0\]
  74:	f2a10f62 	vqrdmlsh.s32	d0, d1, d2\[1\]
  78:	f3a20f42 	vqrdmlsh.s32	q0, q1, d2\[0\]
  7c:	f3a20f62 	vqrdmlsh.s32	q0, q1, d2\[1\]

00000080 <.*>:
  80:	ff11 0b12 	vqrdmlah.s16	d0, d1, d2
  84:	ff12 0b54 	vqrdmlah.s16	q0, q1, q2
  88:	ff21 0b12 	vqrdmlah.s32	d0, d1, d2
  8c:	ff22 0b54 	vqrdmlah.s32	q0, q1, q2
  90:	ff11 0c12 	vqrdmlsh.s16	d0, d1, d2
  94:	ff12 0c54 	vqrdmlsh.s16	q0, q1, q2
  98:	ff21 0c12 	vqrdmlsh.s32	d0, d1, d2
  9c:	ff22 0c54 	vqrdmlsh.s32	q0, q1, q2
  a0:	ef91 0e42 	vqrdmlah.s16	d0, d1, d2\[0\]
  a4:	ef91 0e4a 	vqrdmlah.s16	d0, d1, d2\[1\]
  a8:	ef91 0e62 	vqrdmlah.s16	d0, d1, d2\[2\]
  ac:	ef91 0e6a 	vqrdmlah.s16	d0, d1, d2\[3\]
  b0:	ff92 0e42 	vqrdmlah.s16	q0, q1, d2\[0\]
  b4:	ff92 0e4a 	vqrdmlah.s16	q0, q1, d2\[1\]
  b8:	ff92 0e62 	vqrdmlah.s16	q0, q1, d2\[2\]
  bc:	ff92 0e6a 	vqrdmlah.s16	q0, q1, d2\[3\]
  c0:	efa1 0e42 	vqrdmlah.s32	d0, d1, d2\[0\]
  c4:	efa1 0e62 	vqrdmlah.s32	d0, d1, d2\[1\]
  c8:	ffa2 0e42 	vqrdmlah.s32	q0, q1, d2\[0\]
  cc:	ffa2 0e62 	vqrdmlah.s32	q0, q1, d2\[1\]
  d0:	ef91 0f42 	vqrdmlsh.s16	d0, d1, d2\[0\]
  d4:	ef91 0f4a 	vqrdmlsh.s16	d0, d1, d2\[1\]
  d8:	ef91 0f62 	vqrdmlsh.s16	d0, d1, d2\[2\]
  dc:	ef91 0f6a 	vqrdmlsh.s16	d0, d1, d2\[3\]
  e0:	ff92 0f42 	vqrdmlsh.s16	q0, q1, d2\[0\]
  e4:	ff92 0f4a 	vqrdmlsh.s16	q0, q1, d2\[1\]
  e8:	ff92 0f62 	vqrdmlsh.s16	q0, q1, d2\[2\]
  ec:	ff92 0f6a 	vqrdmlsh.s16	q0, q1, d2\[3\]
  f0:	efa1 0f42 	vqrdmlsh.s32	d0, d1, d2\[0\]
  f4:	efa1 0f62 	vqrdmlsh.s32	d0, d1, d2\[1\]
  f8:	ffa2 0f42 	vqrdmlsh.s32	q0, q1, d2\[0\]
  fc:	ffa2 0f62 	vqrdmlsh.s32	q0, q1, d2\[1\]

