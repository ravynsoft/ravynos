#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d4a001e1 	dcps1	#0xf
   4:	d4a00001 	dcps1
   8:	d4a00001 	dcps1
   c:	d4a003e2 	dcps2	#0x1f
  10:	d4a00002 	dcps2
  14:	d4a00002 	dcps2
  18:	d4a007e3 	dcps3	#0x3f
  1c:	d4a00003 	dcps3
  20:	d4a00003 	dcps3
  24:	d65f00e0 	ret	x7
  28:	d65f03c0 	ret
  2c:	d65f03c0 	ret
  30:	d503305f 	clrex	#0x0
  34:	d503395f 	clrex	#0x9
  38:	d5033f5f 	clrex
  3c:	d5033f5f 	clrex
  40:	d508001f 	sys	#0, C0, C0, #0
  44:	10000000 	adr	x0, 0 <sym>
			44: .*	sym
  48:	f9400001 	ldr	x1, \[x0\]
			48: .*	sym
  4c:	f9400001 	ldr	x1, \[x0\]
			4c: .*	sym
  50:	f9000001 	str	x1, \[x0\]
			50: .*	sym
  54:	f9000001 	str	x1, \[x0\]
			54: .*	sym
