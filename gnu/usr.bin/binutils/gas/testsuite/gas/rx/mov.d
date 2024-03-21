#source: ./mov.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   [0-9a-f]+:	c3 00[ 	]+mov\.b	r0, \[r0\]
   [0-9a-f]+:	c3 70[ 	]+mov\.b	r0, \[r7\]
   [0-9a-f]+:	c3 07[ 	]+mov\.b	r7, \[r0\]
   [0-9a-f]+:	c3 77[ 	]+mov\.b	r7, \[r7\]
   [0-9a-f]+:	d3 00[ 	]+mov\.w	r0, \[r0\]
   [0-9a-f]+:	d3 70[ 	]+mov\.w	r0, \[r7\]
   [0-9a-f]+:	d3 07[ 	]+mov\.w	r7, \[r0\]
   [0-9a-f]+:	d3 77[ 	]+mov\.w	r7, \[r7\]
  [0-9a-f]+:	e3 00[ 	]+mov\.l	r0, \[r0\]
  [0-9a-f]+:	e3 70[ 	]+mov\.l	r0, \[r7\]
  [0-9a-f]+:	e3 07[ 	]+mov\.l	r7, \[r0\]
  [0-9a-f]+:	e3 77[ 	]+mov\.l	r7, \[r7\]
#
  [0-9a-f]+:	80 00[ 	]+mov\.b	r0, 0\[r0\]
  [0-9a-f]+:	80 70[ 	]+mov\.b	r0, 0\[r7\]
  [0-9a-f]+:	80 07[ 	]+mov\.b	r7, 0\[r0\]
  [0-9a-f]+:	80 77[ 	]+mov\.b	r7, 0\[r7\]
  [0-9a-f]+:	90 00[ 	]+mov\.w	r0, 0\[r0\]
  [0-9a-f]+:	90 70[ 	]+mov\.w	r0, 0\[r7\]
  [0-9a-f]+:	90 07[ 	]+mov\.w	r7, 0\[r0\]
  [0-9a-f]+:	90 77[ 	]+mov\.w	r7, 0\[r7\]
  [0-9a-f]+:	a0 00[ 	]+mov\.l	r0, 0\[r0\]
  [0-9a-f]+:	a0 70[ 	]+mov\.l	r0, 0\[r7\]
  [0-9a-f]+:	a0 07[ 	]+mov\.l	r7, 0\[r0\]
  [0-9a-f]+:	a0 77[ 	]+mov\.l	r7, 0\[r7\]
#
  [0-9a-f]+:	81 00                         	mov\.b	r0, 4\[r0\]
  [0-9a-f]+:	81 70                         	mov\.b	r0, 4\[r7\]
  [0-9a-f]+:	87 00                         	mov\.b	r0, 28\[r0\]
  [0-9a-f]+:	87 70                         	mov\.b	r0, 28\[r7\]
  [0-9a-f]+:	81 07                         	mov\.b	r7, 4\[r0\]
  [0-9a-f]+:	81 77                         	mov\.b	r7, 4\[r7\]
  [0-9a-f]+:	87 07                         	mov\.b	r7, 28\[r0\]
  [0-9a-f]+:	87 77                         	mov\.b	r7, 28\[r7\]
  [0-9a-f]+:	90 80                         	mov\.w	r0, 4\[r0\]
  [0-9a-f]+:	90 f0                         	mov\.w	r0, 4\[r7\]
  [0-9a-f]+:	93 80                         	mov\.w	r0, 28\[r0\]
  [0-9a-f]+:	93 f0                         	mov\.w	r0, 28\[r7\]
  [0-9a-f]+:	90 87                         	mov\.w	r7, 4\[r0\]
  [0-9a-f]+:	90 f7                         	mov\.w	r7, 4\[r7\]
  [0-9a-f]+:	93 87                         	mov\.w	r7, 28\[r0\]
  [0-9a-f]+:	93 f7                         	mov\.w	r7, 28\[r7\]
  [0-9a-f]+:	a0 08                         	mov\.l	r0, 4\[r0\]
  [0-9a-f]+:	a0 78                         	mov\.l	r0, 4\[r7\]
  [0-9a-f]+:	a1 88                         	mov\.l	r0, 28\[r0\]
  [0-9a-f]+:	a1 f8                         	mov\.l	r0, 28\[r7\]
  [0-9a-f]+:	a0 0f                         	mov\.l	r7, 4\[r0\]
  [0-9a-f]+:	a0 7f                         	mov\.l	r7, 4\[r7\]
  [0-9a-f]+:	a1 8f                         	mov\.l	r7, 28\[r0\]
  [0-9a-f]+:	a1 ff                         	mov\.l	r7, 28\[r7\]
#
  [0-9a-f]+:	cc 00[ 	]+mov\.b	\[r0\], r0
  [0-9a-f]+:	cc 07[ 	]+mov\.b	\[r0\], r7
  [0-9a-f]+:	cc 70[ 	]+mov\.b	\[r7\], r0
  [0-9a-f]+:	cc 77[ 	]+mov\.b	\[r7\], r7
  [0-9a-f]+:	dc 00[ 	]+mov\.w	\[r0\], r0
  [0-9a-f]+:	dc 07[ 	]+mov\.w	\[r0\], r7
  [0-9a-f]+:	dc 70[ 	]+mov\.w	\[r7\], r0
  [0-9a-f]+:	dc 77[ 	]+mov\.w	\[r7\], r7
  [0-9a-f]+:	ec 00[ 	]+mov\.l	\[r0\], r0
  [0-9a-f]+:	ec 07[ 	]+mov\.l	\[r0\], r7
  [0-9a-f]+:	ec 70[ 	]+mov\.l	\[r7\], r0
  [0-9a-f]+:	ec 77[ 	]+mov\.l	\[r7\], r7
#   
  [0-9a-f]+:	88 00[ 	]+mov\.b	0\[r0\], r0
  [0-9a-f]+:	88 07[ 	]+mov\.b	0\[r0\], r7
  [0-9a-f]+:	88 70[ 	]+mov\.b	0\[r7\], r0
  [0-9a-f]+:	88 77[ 	]+mov\.b	0\[r7\], r7
  [0-9a-f]+:	98 00[ 	]+mov\.w	0\[r0\], r0
  [0-9a-f]+:	98 07[ 	]+mov\.w	0\[r0\], r7
  [0-9a-f]+:	98 70[ 	]+mov\.w	0\[r7\], r0
  [0-9a-f]+:	98 77[ 	]+mov\.w	0\[r7\], r7
  [0-9a-f]+:	a8 00[ 	]+mov\.l	0\[r0\], r0
  [0-9a-f]+:	a8 07[ 	]+mov\.l	0\[r0\], r7
  [0-9a-f]+:	a8 70[ 	]+mov\.l	0\[r7\], r0
  [0-9a-f]+:	a8 77[ 	]+mov\.l	0\[r7\], r7
#   
  [0-9a-f]+:	89 00                         	mov\.b	4\[r0\], r0
  [0-9a-f]+:	89 07                         	mov\.b	4\[r0\], r7
  [0-9a-f]+:	89 70                         	mov\.b	4\[r7\], r0
  [0-9a-f]+:	89 77                         	mov\.b	4\[r7\], r7
  [0-9a-f]+:	8f 00                         	mov\.b	28\[r0\], r0
  [0-9a-f]+:	8f 07                         	mov\.b	28\[r0\], r7
  [0-9a-f]+:	8f 70                         	mov\.b	28\[r7\], r0
  [0-9a-f]+:	8f 77                         	mov\.b	28\[r7\], r7
  [0-9a-f]+:	98 80                         	mov\.w	4\[r0\], r0
  [0-9a-f]+:	98 87                         	mov\.w	4\[r0\], r7
  [0-9a-f]+:	98 f0                         	mov\.w	4\[r7\], r0
  [0-9a-f]+:	98 f7                         	mov\.w	4\[r7\], r7
  [0-9a-f]+:	9b 80                         	mov\.w	28\[r0\], r0
  [0-9a-f]+:	9b 87                         	mov\.w	28\[r0\], r7
  [0-9a-f]+:	9b f0                         	mov\.w	28\[r7\], r0
  [0-9a-f]+:	9b f7                         	mov\.w	28\[r7\], r7
  [0-9a-f]+:	a8 08                         	mov\.l	4\[r0\], r0
  [0-9a-f]+:	a8 0f                         	mov\.l	4\[r0\], r7
  [0-9a-f]+:	a8 78                         	mov\.l	4\[r7\], r0
  [0-9a-f]+:	a8 7f                         	mov\.l	4\[r7\], r7
  [0-9a-f]+:	a9 88                         	mov\.l	28\[r0\], r0
  [0-9a-f]+:	a9 8f                         	mov\.l	28\[r0\], r7
  [0-9a-f]+:	a9 f8                         	mov\.l	28\[r7\], r0
  [0-9a-f]+:	a9 ff                         	mov\.l	28\[r7\], r7
  [0-9a-f]+:	66 00                         	mov\.l	#0, r0
  [0-9a-f]+:	66 0f                         	mov\.l	#0, r15
  [0-9a-f]+:	66 f0                         	mov\.l	#15, r0
  [0-9a-f]+:	66 ff                         	mov\.l	#15, r15
  [0-9a-f]+:	f9 04 04 80                   	mov\.b	#128, 4\[r0\]
  [0-9a-f]+:	f9 74 04 80                   	mov\.b	#128, 4\[r7\]
  [0-9a-f]+:	f9 04 1c 80                   	mov\.b	#128, 28\[r0\]
  [0-9a-f]+:	f9 74 1c 80                   	mov\.b	#128, 28\[r7\]
  [0-9a-f]+:	3c 04 7f                      	mov\.b	#127, 4\[r0\]
  [0-9a-f]+:	3c 74 7f                      	mov\.b	#127, 4\[r7\]
  [0-9a-f]+:	3c 8c 7f                      	mov\.b	#127, 28\[r0\]
  [0-9a-f]+:	3c fc 7f                      	mov\.b	#127, 28\[r7\]
  [0-9a-f]+:	3d 02 00                      	mov\.w	#0, 4\[r0\]
  [0-9a-f]+:	3d 72 00                      	mov\.w	#0, 4\[r7\]
  [0-9a-f]+:	3d 0e 00                      	mov\.w	#0, 28\[r0\]
  [0-9a-f]+:	3d 7e 00                      	mov\.w	#0, 28\[r7\]
  [0-9a-f]+:	3d 02 ff                      	mov\.w	#255, 4\[r0\]
  [0-9a-f]+:	3d 72 ff                      	mov\.w	#255, 4\[r7\]
  [0-9a-f]+:	3d 0e ff                      	mov\.w	#255, 28\[r0\]
  [0-9a-f]+:	3d 7e ff                      	mov\.w	#255, 28\[r7\]
  [0-9a-f]+:	3e 01 00                      	mov\.l	#0, 4\[r0\]
  [0-9a-f]+:	3e 71 00                      	mov\.l	#0, 4\[r7\]
 [0-9a-f]+:	3e 07 00                      	mov\.l	#0, 28\[r0\]
 [0-9a-f]+:	3e 77 00                      	mov\.l	#0, 28\[r7\]
 [0-9a-f]+:	3e 01 ff                      	mov\.l	#255, 4\[r0\]
 [0-9a-f]+:	3e 71 ff                      	mov\.l	#255, 4\[r7\]
 [0-9a-f]+:	3e 07 ff                      	mov\.l	#255, 28\[r0\]
 [0-9a-f]+:	3e 77 ff                      	mov\.l	#255, 28\[r7\]
 [0-9a-f]+:	66 00                         	mov\.l	#0, r0
 [0-9a-f]+:	66 0f                         	mov\.l	#0, r15
 [0-9a-f]+:	75 40 ff                      	mov\.l	#255, r0
 [0-9a-f]+:	75 4f ff                      	mov\.l	#255, r15
 [0-9a-f]+:	fb 06 80                      	mov\.l	#-128, r0
 [0-9a-f]+:	fb f6 80                      	mov\.l	#-128, r15
 [0-9a-f]+:	75 40 7f                      	mov\.l	#127, r0
 [0-9a-f]+:	75 4f 7f                      	mov\.l	#127, r15
 [0-9a-f]+:	fb 0a 00 80                   	mov\.l	#0xffff8000, r0
 [0-9a-f]+:	fb fa 00 80                   	mov\.l	#0xffff8000, r15
 [0-9a-f]+:	fb 0e 00 80 00                	mov\.l	#0x8000, r0
 [0-9a-f]+:	fb fe 00 80 00                	mov\.l	#0x8000, r15
 [0-9a-f]+:	fb 0e 00 00 80                	mov\.l	#0xff800000, r0
 [0-9a-f]+:	fb fe 00 00 80                	mov\.l	#0xff800000, r15
 [0-9a-f]+:	fb 0e ff ff 7f                	mov\.l	#0x7fffff, r0
 [0-9a-f]+:	fb fe ff ff 7f                	mov\.l	#0x7fffff, r15
 [0-9a-f]+:	fb 02 00 00 00 80             	mov\.l	#0x80000000, r0
 [0-9a-f]+:	fb f2 00 00 00 80             	mov\.l	#0x80000000, r15
 [0-9a-f]+:	fb 02 ff ff ff 7f             	mov\.l	#0x7fffffff, r0
 [0-9a-f]+:	fb f2 ff ff ff 7f             	mov\.l	#0x7fffffff, r15
 [0-9a-f]+:	cf 10                         	mov\.b	r1, r0
 [0-9a-f]+:	cf 1f                         	mov\.b	r1, r15
 [0-9a-f]+:	cf f0                         	mov\.b	r15, r0
 [0-9a-f]+:	cf ff                         	mov\.b	r15, r15
 [0-9a-f]+:	df 10                         	mov\.w	r1, r0
 [0-9a-f]+:	df 1f                         	mov\.w	r1, r15
 [0-9a-f]+:	df f0                         	mov\.w	r15, r0
 [0-9a-f]+:	df ff                         	mov\.w	r15, r15
 [0-9a-f]+:	ef 10                         	mov\.l	r1, r0
 [0-9a-f]+:	ef 1f                         	mov\.l	r1, r15
 [0-9a-f]+:	ef f0                         	mov\.l	r15, r0
 [0-9a-f]+:	ef ff                         	mov\.l	r15, r15
 [0-9a-f]+:	f8 04 00                      	mov\.b	#0, \[r0\]
 [0-9a-f]+:	f8 f4 00                      	mov\.b	#0, \[r15\]
 [0-9a-f]+:	f9 04 fc 00                   	mov\.b	#0, 252\[r0\]
 [0-9a-f]+:	f9 f4 fc 00                   	mov\.b	#0, 252\[r15\]
 [0-9a-f]+:	fa 04 fc ff 00                	mov\.b	#0, 65532\[r0\]
 [0-9a-f]+:	fa f4 fc ff 00                	mov\.b	#0, 65532\[r15\]
 [0-9a-f]+:	f8 04 ff                      	mov\.b	#255, \[r0\]
 [0-9a-f]+:	f8 f4 ff                      	mov\.b	#255, \[r15\]
 [0-9a-f]+:	f9 04 fc ff                   	mov\.b	#255, 252\[r0\]
 [0-9a-f]+:	f9 f4 fc ff                   	mov\.b	#255, 252\[r15\]
 [0-9a-f]+:	fa 04 fc ff ff                	mov\.b	#255, 65532\[r0\]
 [0-9a-f]+:	fa f4 fc ff ff                	mov\.b	#255, 65532\[r15\]
 [0-9a-f]+:	f8 05 80                      	mov\.w	#-128, \[r0\]
 [0-9a-f]+:	f8 f5 80                      	mov\.w	#-128, \[r15\]
 [0-9a-f]+:	f9 05 7e 80                   	mov\.w	#-128, 252\[r0\]
 [0-9a-f]+:	f9 f5 7e 80                   	mov\.w	#-128, 252\[r15\]
 [0-9a-f]+:	fa 05 fe 7f 80                	mov\.w	#-128, 65532\[r0\]
 [0-9a-f]+:	fa f5 fe 7f 80                	mov\.w	#-128, 65532\[r15\]
 [0-9a-f]+:	f8 05 7f                      	mov\.w	#127, \[r0\]
 [0-9a-f]+:	f8 f5 7f                      	mov\.w	#127, \[r15\]
 [0-9a-f]+:	f9 05 7e 7f                   	mov\.w	#127, 252\[r0\]
 [0-9a-f]+:	f9 f5 7e 7f                   	mov\.w	#127, 252\[r15\]
 [0-9a-f]+:	fa 05 fe 7f 7f                	mov\.w	#127, 65532\[r0\]
 [0-9a-f]+:	fa f5 fe 7f 7f                	mov\.w	#127, 65532\[r15\]
 [0-9a-f]+:	f8 05 00                      	mov\.w	#0, \[r0\]
 [0-9a-f]+:	f8 f5 00                      	mov\.w	#0, \[r15\]
 [0-9a-f]+:	f9 05 7e 00                   	mov\.w	#0, 252\[r0\]
 [0-9a-f]+:	f9 f5 7e 00                   	mov\.w	#0, 252\[r15\]
 [0-9a-f]+:	fa 05 fe 7f 00                	mov\.w	#0, 65532\[r0\]
 [0-9a-f]+:	fa f5 fe 7f 00                	mov\.w	#0, 65532\[r15\]
 [0-9a-f]+:	f8 05 ff                      	mov.w	#-1, \[r0\]
 [0-9a-f]+:	f8 f5 ff                      	mov.w	#-1, \[r15\]
 [0-9a-f]+:	f9 05 7e ff                   	mov.w	#-1, 252\[r0\]
 [0-9a-f]+:	f9 f5 7e ff                   	mov.w	#-1, 252\[r15\]
 [0-9a-f]+:	fa 05 fe 7f ff                	mov.w	#-1, 65532\[r0\]
 [0-9a-f]+:	fa f5 fe 7f ff                	mov.w	#-1, 65532\[r15\]
 [0-9a-f]+:	f8 06 80                      	mov.l	#-128, \[r0\]
 [0-9a-f]+:	f8 f6 80                      	mov.l	#-128, \[r15\]
 [0-9a-f]+:	f9 06 3f 80                   	mov.l	#-128, 252\[r0\]
 [0-9a-f]+:	f9 f6 3f 80                   	mov.l	#-128, 252\[r15\]
 [0-9a-f]+:	fa 06 ff 3f 80                	mov.l	#-128, 65532\[r0\]
 [0-9a-f]+:	fa f6 ff 3f 80                	mov.l	#-128, 65532\[r15\]
 [0-9a-f]+:	f8 06 7f                      	mov.l	#127, \[r0\]
 [0-9a-f]+:	f8 f6 7f                      	mov.l	#127, \[r15\]
 [0-9a-f]+:	f9 06 3f 7f                   	mov.l	#127, 252\[r0\]
 [0-9a-f]+:	f9 f6 3f 7f                   	mov.l	#127, 252\[r15\]
 [0-9a-f]+:	fa 06 ff 3f 7f                	mov.l	#127, 65532\[r0\]
 [0-9a-f]+:	fa f6 ff 3f 7f                	mov.l	#127, 65532\[r15\]
 [0-9a-f]+:	f8 0a 00 80                   	mov.l	#0xffff8000, \[r0\]
 [0-9a-f]+:	f8 fa 00 80                   	mov.l	#0xffff8000, \[r15\]
 [0-9a-f]+:	f9 0a 3f 00 80                	mov.l	#0xffff8000, 252\[r0\]
 [0-9a-f]+:	f9 fa 3f 00 80                	mov.l	#0xffff8000, 252\[r15\]
 [0-9a-f]+:	fa 0a ff 3f 00 80             	mov.l	#0xffff8000, 65532\[r0\]
 [0-9a-f]+:	fa fa ff 3f 00 80             	mov.l	#0xffff8000, 65532\[r15\]
 [0-9a-f]+:	f8 0e 00 80 00                	mov.l	#0x8000, \[r0\]
 [0-9a-f]+:	f8 fe 00 80 00                	mov.l	#0x8000, \[r15\]
 [0-9a-f]+:	f9 0e 3f 00 80 00             	mov.l	#0x8000, 252\[r0\]
 [0-9a-f]+:	f9 fe 3f 00 80 00             	mov.l	#0x8000, 252\[r15\]
 [0-9a-f]+:	fa 0e ff 3f 00 80 00          	mov.l	#0x8000, 65532\[r0\]
 [0-9a-f]+:	fa fe ff 3f 00 80 00          	mov.l	#0x8000, 65532\[r15\]
 [0-9a-f]+:	f8 0e 00 00 80                	mov.l	#0xff800000, \[r0\]
 [0-9a-f]+:	f8 fe 00 00 80                	mov.l	#0xff800000, \[r15\]
 [0-9a-f]+:	f9 0e 3f 00 00 80             	mov.l	#0xff800000, 252\[r0\]
 [0-9a-f]+:	f9 fe 3f 00 00 80             	mov.l	#0xff800000, 252\[r15\]
 [0-9a-f]+:	fa 0e ff 3f 00 00 80          	mov.l	#0xff800000, 65532\[r0\]
 [0-9a-f]+:	fa fe ff 3f 00 00 80          	mov.l	#0xff800000, 65532\[r15\]
 [0-9a-f]+:	f8 0e ff ff 7f                	mov.l	#0x7fffff, \[r0\]
 [0-9a-f]+:	f8 fe ff ff 7f                	mov.l	#0x7fffff, \[r15\]
 [0-9a-f]+:	f9 0e 3f ff ff 7f             	mov.l	#0x7fffff, 252\[r0\]
 [0-9a-f]+:	f9 fe 3f ff ff 7f             	mov.l	#0x7fffff, 252\[r15\]
 [0-9a-f]+:	fa 0e ff 3f ff ff 7f          	mov.l	#0x7fffff, 65532\[r0\]
 [0-9a-f]+:	fa fe ff 3f ff ff 7f          	mov.l	#0x7fffff, 65532\[r15\]
 [0-9a-f]+:	f8 02 00 00 00 80             	mov.l	#0x80000000, \[r0\]
 [0-9a-f]+:	f8 f2 00 00 00 80             	mov.l	#0x80000000, \[r15\]
 [0-9a-f]+:	f9 02 3f 00 00 00 80          	mov.l	#0x80000000, 252\[r0\]
 [0-9a-f]+:	f9 f2 3f 00 00 00 80          	mov.l	#0x80000000, 252\[r15\]
 [0-9a-f]+:	fa 02 ff 3f 00 00 00 80       	mov.l	#0x80000000, 65532\[r0\]
 [0-9a-f]+:	fa f2 ff 3f 00 00 00 80       	mov.l	#0x80000000, 65532\[r15\]
 [0-9a-f]+:	f8 02 ff ff ff 7f             	mov.l	#0x7fffffff, \[r0\]
 [0-9a-f]+:	f8 f2 ff ff ff 7f             	mov.l	#0x7fffffff, \[r15\]
 [0-9a-f]+:	f9 02 3f ff ff ff 7f          	mov.l	#0x7fffffff, 252\[r0\]
 [0-9a-f]+:	f9 f2 3f ff ff ff 7f          	mov.l	#0x7fffffff, 252\[r15\]
 [0-9a-f]+:	fa 02 ff 3f ff ff ff 7f       	mov.l	#0x7fffffff, 65532\[r0\]
 [0-9a-f]+:	fa f2 ff 3f ff ff ff 7f       	mov.l	#0x7fffffff, 65532\[r15\]
 [0-9a-f]+:	cc 00                         	mov.b	\[r0\], r0
 [0-9a-f]+:	cc 0f                         	mov.b	\[r0\], r15
 [0-9a-f]+:	cc f0                         	mov.b	\[r15\], r0
 [0-9a-f]+:	cc ff                         	mov.b	\[r15\], r15
 [0-9a-f]+:	cd 00 fc                      	mov.b	252\[r0\], r0
 [0-9a-f]+:	cd 0f fc                      	mov.b	252\[r0\], r15
 [0-9a-f]+:	cd f0 fc                      	mov.b	252\[r15\], r0
 [0-9a-f]+:	cd ff fc                      	mov.b	252\[r15\], r15
 [0-9a-f]+:	ce 00 fc ff                   	mov.b	65532\[r0\], r0
 [0-9a-f]+:	ce 0f fc ff                   	mov.b	65532\[r0\], r15
 [0-9a-f]+:	ce f0 fc ff                   	mov.b	65532\[r15\], r0
 [0-9a-f]+:	ce ff fc ff                   	mov.b	65532\[r15\], r15
 [0-9a-f]+:	dc 00                         	mov.w	\[r0\], r0
 [0-9a-f]+:	dc 0f                         	mov.w	\[r0\], r15
 [0-9a-f]+:	dc f0                         	mov.w	\[r15\], r0
 [0-9a-f]+:	dc ff                         	mov.w	\[r15\], r15
 [0-9a-f]+:	dd 00 7e                      	mov.w	252\[r0\], r0
 [0-9a-f]+:	dd 0f 7e                      	mov.w	252\[r0\], r15
 [0-9a-f]+:	dd f0 7e                      	mov.w	252\[r15\], r0
 [0-9a-f]+:	dd ff 7e                      	mov.w	252\[r15\], r15
 [0-9a-f]+:	de 00 fe 7f                   	mov.w	65532\[r0\], r0
 [0-9a-f]+:	de 0f fe 7f                   	mov.w	65532\[r0\], r15
 [0-9a-f]+:	de f0 fe 7f                   	mov.w	65532\[r15\], r0
 [0-9a-f]+:	de ff fe 7f                   	mov.w	65532\[r15\], r15
 [0-9a-f]+:	ec 00                         	mov.l	\[r0\], r0
 [0-9a-f]+:	ec 0f                         	mov.l	\[r0\], r15
 [0-9a-f]+:	ec f0                         	mov.l	\[r15\], r0
 [0-9a-f]+:	ec ff                         	mov.l	\[r15\], r15
 [0-9a-f]+:	ed 00 3f                      	mov.l	252\[r0\], r0
 [0-9a-f]+:	ed 0f 3f                      	mov.l	252\[r0\], r15
 [0-9a-f]+:	ed f0 3f                      	mov.l	252\[r15\], r0
 [0-9a-f]+:	ed ff 3f                      	mov.l	252\[r15\], r15
 [0-9a-f]+:	ee 00 ff 3f                   	mov.l	65532\[r0\], r0
 [0-9a-f]+:	ee 0f ff 3f                   	mov.l	65532\[r0\], r15
 [0-9a-f]+:	ee f0 ff 3f                   	mov.l	65532\[r15\], r0
 [0-9a-f]+:	ee ff ff 3f                   	mov.l	65532\[r15\], r15
 [0-9a-f]+:	fe 40 00                      	mov.b	\[r0, r0\], r0
 [0-9a-f]+:	fe 40 0f                      	mov.b	\[r0, r0\], r15
 [0-9a-f]+:	fe 40 f0                      	mov.b	\[r0, r15\], r0
 [0-9a-f]+:	fe 40 ff                      	mov.b	\[r0, r15\], r15
 [0-9a-f]+:	fe 4f 00                      	mov.b	\[r15, r0\], r0
 [0-9a-f]+:	fe 4f 0f                      	mov.b	\[r15, r0\], r15
 [0-9a-f]+:	fe 4f f0                      	mov.b	\[r15, r15\], r0
 [0-9a-f]+:	fe 4f ff                      	mov.b	\[r15, r15\], r15
 [0-9a-f]+:	fe 50 00                      	mov.w	\[r0, r0\], r0
 [0-9a-f]+:	fe 50 0f                      	mov.w	\[r0, r0\], r15
 [0-9a-f]+:	fe 50 f0                      	mov.w	\[r0, r15\], r0
 [0-9a-f]+:	fe 50 ff                      	mov.w	\[r0, r15\], r15
 [0-9a-f]+:	fe 5f 00                      	mov.w	\[r15, r0\], r0
 [0-9a-f]+:	fe 5f 0f                      	mov.w	\[r15, r0\], r15
 [0-9a-f]+:	fe 5f f0                      	mov.w	\[r15, r15\], r0
 [0-9a-f]+:	fe 5f ff                      	mov.w	\[r15, r15\], r15
 [0-9a-f]+:	fe 60 00                      	mov.l	\[r0, r0\], r0
 [0-9a-f]+:	fe 60 0f                      	mov.l	\[r0, r0\], r15
 [0-9a-f]+:	fe 60 f0                      	mov.l	\[r0, r15\], r0
 [0-9a-f]+:	fe 60 ff                      	mov.l	\[r0, r15\], r15
 [0-9a-f]+:	fe 6f 00                      	mov.l	\[r15, r0\], r0
 [0-9a-f]+:	fe 6f 0f                      	mov.l	\[r15, r0\], r15
 [0-9a-f]+:	fe 6f f0                      	mov.l	\[r15, r15\], r0
 [0-9a-f]+:	fe 6f ff                      	mov.l	\[r15, r15\], r15
 [0-9a-f]+:	c3 01                         	mov.b	r1, \[r0\]
 [0-9a-f]+:	c3 f1                         	mov.b	r1, \[r15\]
 [0-9a-f]+:	c7 01 fc                      	mov.b	r1, 252\[r0\]
 [0-9a-f]+:	c7 f1 fc                      	mov.b	r1, 252\[r15\]
 [0-9a-f]+:	cb 01 fc ff                   	mov.b	r1, 65532\[r0\]
 [0-9a-f]+:	cb f1 fc ff                   	mov.b	r1, 65532\[r15\]
 [0-9a-f]+:	c3 0f                         	mov.b	r15, \[r0\]
 [0-9a-f]+:	c3 ff                         	mov.b	r15, \[r15\]
 [0-9a-f]+:	c7 0f fc                      	mov.b	r15, 252\[r0\]
 [0-9a-f]+:	c7 ff fc                      	mov.b	r15, 252\[r15\]
 [0-9a-f]+:	cb 0f fc ff                   	mov.b	r15, 65532\[r0\]
 [0-9a-f]+:	cb ff fc ff                   	mov.b	r15, 65532\[r15\]
 [0-9a-f]+:	d3 01                         	mov.w	r1, \[r0\]
 [0-9a-f]+:	d3 f1                         	mov.w	r1, \[r15\]
 [0-9a-f]+:	d7 01 7e                      	mov.w	r1, 252\[r0\]
 [0-9a-f]+:	d7 f1 7e                      	mov.w	r1, 252\[r15\]
 [0-9a-f]+:	db 01 fe 7f                   	mov.w	r1, 65532\[r0\]
 [0-9a-f]+:	db f1 fe 7f                   	mov.w	r1, 65532\[r15\]
 [0-9a-f]+:	d3 0f                         	mov.w	r15, \[r0\]
 [0-9a-f]+:	d3 ff                         	mov.w	r15, \[r15\]
 [0-9a-f]+:	d7 0f 7e                      	mov.w	r15, 252\[r0\]
 [0-9a-f]+:	d7 ff 7e                      	mov.w	r15, 252\[r15\]
 [0-9a-f]+:	db 0f fe 7f                   	mov.w	r15, 65532\[r0\]
 [0-9a-f]+:	db ff fe 7f                   	mov.w	r15, 65532\[r15\]
 [0-9a-f]+:	e3 01                         	mov.l	r1, \[r0\]
 [0-9a-f]+:	e3 f1                         	mov.l	r1, \[r15\]
 [0-9a-f]+:	e7 01 3f                      	mov.l	r1, 252\[r0\]
 [0-9a-f]+:	e7 f1 3f                      	mov.l	r1, 252\[r15\]
 [0-9a-f]+:	eb 01 ff 3f                   	mov.l	r1, 65532\[r0\]
 [0-9a-f]+:	eb f1 ff 3f                   	mov.l	r1, 65532\[r15\]
 [0-9a-f]+:	e3 0f                         	mov.l	r15, \[r0\]
 [0-9a-f]+:	e3 ff                         	mov.l	r15, \[r15\]
 [0-9a-f]+:	e7 0f 3f                      	mov.l	r15, 252\[r0\]
 [0-9a-f]+:	e7 ff 3f                      	mov.l	r15, 252\[r15\]
 [0-9a-f]+:	eb 0f ff 3f                   	mov.l	r15, 65532\[r0\]
 [0-9a-f]+:	eb ff ff 3f                   	mov.l	r15, 65532\[r15\]
 [0-9a-f]+:	fe 00 00                      	mov.b	r0, \[r0, r0\]
 [0-9a-f]+:	fe 00 f0                      	mov.b	r0, \[r0, r15\]
 [0-9a-f]+:	fe 0f 00                      	mov.b	r0, \[r15, r0\]
 [0-9a-f]+:	fe 0f f0                      	mov.b	r0, \[r15, r15\]
 [0-9a-f]+:	fe 00 0f                      	mov.b	r15, \[r0, r0\]
 [0-9a-f]+:	fe 00 ff                      	mov.b	r15, \[r0, r15\]
 [0-9a-f]+:	fe 0f 0f                      	mov.b	r15, \[r15, r0\]
 [0-9a-f]+:	fe 0f ff                      	mov.b	r15, \[r15, r15\]
 [0-9a-f]+:	fe 10 00                      	mov.w	r0, \[r0, r0\]
 [0-9a-f]+:	fe 10 f0                      	mov.w	r0, \[r0, r15\]
 [0-9a-f]+:	fe 1f 00                      	mov.w	r0, \[r15, r0\]
 [0-9a-f]+:	fe 1f f0                      	mov.w	r0, \[r15, r15\]
 [0-9a-f]+:	fe 10 0f                      	mov.w	r15, \[r0, r0\]
 [0-9a-f]+:	fe 10 ff                      	mov.w	r15, \[r0, r15\]
 [0-9a-f]+:	fe 1f 0f                      	mov.w	r15, \[r15, r0\]
 [0-9a-f]+:	fe 1f ff                      	mov.w	r15, \[r15, r15\]
 [0-9a-f]+:	fe 20 00                      	mov.l	r0, \[r0, r0\]
 [0-9a-f]+:	fe 20 f0                      	mov.l	r0, \[r0, r15\]
 [0-9a-f]+:	fe 2f 00                      	mov.l	r0, \[r15, r0\]
 [0-9a-f]+:	fe 2f f0                      	mov.l	r0, \[r15, r15\]
 [0-9a-f]+:	fe 20 0f                      	mov.l	r15, \[r0, r0\]
 [0-9a-f]+:	fe 20 ff                      	mov.l	r15, \[r0, r15\]
 [0-9a-f]+:	fe 2f 0f                      	mov.l	r15, \[r15, r0\]
 [0-9a-f]+:	fe 2f ff                      	mov.l	r15, \[r15, r15\]
 [0-9a-f]+:	c0 00                         	mov.b	\[r0\], \[r0\]
 [0-9a-f]+:	c0 0f                         	mov.b	\[r0\], \[r15\]
 [0-9a-f]+:	c4 00 fc                      	mov.b	\[r0\], 252\[r0\]
 [0-9a-f]+:	c4 0f fc                      	mov.b	\[r0\], 252\[r15\]
 [0-9a-f]+:	c8 00 fc ff                   	mov.b	\[r0\], 65532\[r0\]
 [0-9a-f]+:	c8 0f fc ff                   	mov.b	\[r0\], 65532\[r15\]
 [0-9a-f]+:	c0 f0                         	mov.b	\[r15\], \[r0\]
 [0-9a-f]+:	c0 ff                         	mov.b	\[r15\], \[r15\]
 [0-9a-f]+:	c4 f0 fc                      	mov.b	\[r15\], 252\[r0\]
 [0-9a-f]+:	c4 ff fc                      	mov.b	\[r15\], 252\[r15\]
 [0-9a-f]+:	c8 f0 fc ff                   	mov.b	\[r15\], 65532\[r0\]
 [0-9a-f]+:	c8 ff fc ff                   	mov.b	\[r15\], 65532\[r15\]
 [0-9a-f]+:	c1 00 fc                      	mov.b	252\[r0\], \[r0\]
 [0-9a-f]+:	c1 0f fc                      	mov.b	252\[r0\], \[r15\]
 [0-9a-f]+:	c5 00 fc fc                   	mov.b	252\[r0\], 252\[r0\]
 [0-9a-f]+:	c5 0f fc fc                   	mov.b	252\[r0\], 252\[r15\]
 [0-9a-f]+:	c9 00 fc fc ff                	mov.b	252\[r0\], 65532\[r0\]
 [0-9a-f]+:	c9 0f fc fc ff                	mov.b	252\[r0\], 65532\[r15\]
 [0-9a-f]+:	c1 f0 fc                      	mov.b	252\[r15\], \[r0\]
 [0-9a-f]+:	c1 ff fc                      	mov.b	252\[r15\], \[r15\]
 [0-9a-f]+:	c5 f0 fc fc                   	mov.b	252\[r15\], 252\[r0\]
 [0-9a-f]+:	c5 ff fc fc                   	mov.b	252\[r15\], 252\[r15\]
 [0-9a-f]+:	c9 f0 fc fc ff                	mov.b	252\[r15\], 65532\[r0\]
 [0-9a-f]+:	c9 ff fc fc ff                	mov.b	252\[r15\], 65532\[r15\]
 [0-9a-f]+:	c2 00 fc ff                   	mov.b	65532\[r0\], \[r0\]
 [0-9a-f]+:	c2 0f fc ff                   	mov.b	65532\[r0\], \[r15\]
 [0-9a-f]+:	c6 00 fc ff fc                	mov.b	65532\[r0\], 252\[r0\]
 [0-9a-f]+:	c6 0f fc ff fc                	mov.b	65532\[r0\], 252\[r15\]
 [0-9a-f]+:	ca 00 fc ff fc ff             	mov.b	65532\[r0\], 65532\[r0\]
 [0-9a-f]+:	ca 0f fc ff fc ff             	mov.b	65532\[r0\], 65532\[r15\]
 [0-9a-f]+:	c2 f0 fc ff                   	mov.b	65532\[r15\], \[r0\]
 [0-9a-f]+:	c2 ff fc ff                   	mov.b	65532\[r15\], \[r15\]
 [0-9a-f]+:	c6 f0 fc ff fc                	mov.b	65532\[r15\], 252\[r0\]
 [0-9a-f]+:	c6 ff fc ff fc                	mov.b	65532\[r15\], 252\[r15\]
 [0-9a-f]+:	ca f0 fc ff fc ff             	mov.b	65532\[r15\], 65532\[r0\]
 [0-9a-f]+:	ca ff fc ff fc ff             	mov.b	65532\[r15\], 65532\[r15\]
 [0-9a-f]+:	d0 00                         	mov.w	\[r0\], \[r0\]
 [0-9a-f]+:	d0 0f                         	mov.w	\[r0\], \[r15\]
 [0-9a-f]+:	d4 00 7e                      	mov.w	\[r0\], 252\[r0\]
 [0-9a-f]+:	d4 0f 7e                      	mov.w	\[r0\], 252\[r15\]
 [0-9a-f]+:	d8 00 fe 7f                   	mov.w	\[r0\], 65532\[r0\]
 [0-9a-f]+:	d8 0f fe 7f                   	mov.w	\[r0\], 65532\[r15\]
 [0-9a-f]+:	d0 f0                         	mov.w	\[r15\], \[r0\]
 [0-9a-f]+:	d0 ff                         	mov.w	\[r15\], \[r15\]
 [0-9a-f]+:	d4 f0 7e                      	mov.w	\[r15\], 252\[r0\]
 [0-9a-f]+:	d4 ff 7e                      	mov.w	\[r15\], 252\[r15\]
 [0-9a-f]+:	d8 f0 fe 7f                   	mov.w	\[r15\], 65532\[r0\]
 [0-9a-f]+:	d8 ff fe 7f                   	mov.w	\[r15\], 65532\[r15\]
 [0-9a-f]+:	d1 00 7e                      	mov.w	252\[r0\], \[r0\]
 [0-9a-f]+:	d1 0f 7e                      	mov.w	252\[r0\], \[r15\]
 [0-9a-f]+:	d5 00 7e 7e                   	mov.w	252\[r0\], 252\[r0\]
 [0-9a-f]+:	d5 0f 7e 7e                   	mov.w	252\[r0\], 252\[r15\]
 [0-9a-f]+:	d9 00 7e fe 7f                	mov.w	252\[r0\], 65532\[r0\]
 [0-9a-f]+:	d9 0f 7e fe 7f                	mov.w	252\[r0\], 65532\[r15\]
 [0-9a-f]+:	d1 f0 7e                      	mov.w	252\[r15\], \[r0\]
 [0-9a-f]+:	d1 ff 7e                      	mov.w	252\[r15\], \[r15\]
 [0-9a-f]+:	d5 f0 7e 7e                   	mov.w	252\[r15\], 252\[r0\]
 [0-9a-f]+:	d5 ff 7e 7e                   	mov.w	252\[r15\], 252\[r15\]
 [0-9a-f]+:	d9 f0 7e fe 7f                	mov.w	252\[r15\], 65532\[r0\]
 [0-9a-f]+:	d9 ff 7e fe 7f                	mov.w	252\[r15\], 65532\[r15\]
 [0-9a-f]+:	d2 00 fe 7f                   	mov.w	65532\[r0\], \[r0\]
 [0-9a-f]+:	d2 0f fe 7f                   	mov.w	65532\[r0\], \[r15\]
 [0-9a-f]+:	d6 00 fe 7f 7e                	mov.w	65532\[r0\], 252\[r0\]
 [0-9a-f]+:	d6 0f fe 7f 7e                	mov.w	65532\[r0\], 252\[r15\]
 [0-9a-f]+:	da 00 fe 7f fe 7f             	mov.w	65532\[r0\], 65532\[r0\]
 [0-9a-f]+:	da 0f fe 7f fe 7f             	mov.w	65532\[r0\], 65532\[r15\]
 [0-9a-f]+:	d2 f0 fe 7f                   	mov.w	65532\[r15\], \[r0\]
 [0-9a-f]+:	d2 ff fe 7f                   	mov.w	65532\[r15\], \[r15\]
 [0-9a-f]+:	d6 f0 fe 7f 7e                	mov.w	65532\[r15\], 252\[r0\]
 [0-9a-f]+:	d6 ff fe 7f 7e                	mov.w	65532\[r15\], 252\[r15\]
 [0-9a-f]+:	da f0 fe 7f fe 7f             	mov.w	65532\[r15\], 65532\[r0\]
 [0-9a-f]+:	da ff fe 7f fe 7f             	mov.w	65532\[r15\], 65532\[r15\]
 [0-9a-f]+:	e0 00                         	mov.l	\[r0\], \[r0\]
 [0-9a-f]+:	e0 0f                         	mov.l	\[r0\], \[r15\]
 [0-9a-f]+:	e4 00 3f                      	mov.l	\[r0\], 252\[r0\]
 [0-9a-f]+:	e4 0f 3f                      	mov.l	\[r0\], 252\[r15\]
 [0-9a-f]+:	e8 00 ff 3f                   	mov.l	\[r0\], 65532\[r0\]
 [0-9a-f]+:	e8 0f ff 3f                   	mov.l	\[r0\], 65532\[r15\]
 [0-9a-f]+:	e0 f0                         	mov.l	\[r15\], \[r0\]
 [0-9a-f]+:	e0 ff                         	mov.l	\[r15\], \[r15\]
 [0-9a-f]+:	e4 f0 3f                      	mov.l	\[r15\], 252\[r0\]
 [0-9a-f]+:	e4 ff 3f                      	mov.l	\[r15\], 252\[r15\]
 [0-9a-f]+:	e8 f0 ff 3f                   	mov.l	\[r15\], 65532\[r0\]
 [0-9a-f]+:	e8 ff ff 3f                   	mov.l	\[r15\], 65532\[r15\]
 [0-9a-f]+:	e1 00 3f                      	mov.l	252\[r0\], \[r0\]
 [0-9a-f]+:	e1 0f 3f                      	mov.l	252\[r0\], \[r15\]
 [0-9a-f]+:	e5 00 3f 3f                   	mov.l	252\[r0\], 252\[r0\]
 [0-9a-f]+:	e5 0f 3f 3f                   	mov.l	252\[r0\], 252\[r15\]
 [0-9a-f]+:	e9 00 3f ff 3f                	mov.l	252\[r0\], 65532\[r0\]
 [0-9a-f]+:	e9 0f 3f ff 3f                	mov.l	252\[r0\], 65532\[r15\]
 [0-9a-f]+:	e1 f0 3f                      	mov.l	252\[r15\], \[r0\]
 [0-9a-f]+:	e1 ff 3f                      	mov.l	252\[r15\], \[r15\]
 [0-9a-f]+:	e5 f0 3f 3f                   	mov.l	252\[r15\], 252\[r0\]
 [0-9a-f]+:	e5 ff 3f 3f                   	mov.l	252\[r15\], 252\[r15\]
 [0-9a-f]+:	e9 f0 3f ff 3f                	mov.l	252\[r15\], 65532\[r0\]
 [0-9a-f]+:	e9 ff 3f ff 3f                	mov.l	252\[r15\], 65532\[r15\]
 [0-9a-f]+:	e2 00 ff 3f                   	mov.l	65532\[r0\], \[r0\]
 [0-9a-f]+:	e2 0f ff 3f                   	mov.l	65532\[r0\], \[r15\]
 [0-9a-f]+:	e6 00 ff 3f 3f                	mov.l	65532\[r0\], 252\[r0\]
 [0-9a-f]+:	e6 0f ff 3f 3f                	mov.l	65532\[r0\], 252\[r15\]
 [0-9a-f]+:	ea 00 ff 3f ff 3f             	mov.l	65532\[r0\], 65532\[r0\]
 [0-9a-f]+:	ea 0f ff 3f ff 3f             	mov.l	65532\[r0\], 65532\[r15\]
 [0-9a-f]+:	e2 f0 ff 3f                   	mov.l	65532\[r15\], \[r0\]
 [0-9a-f]+:	e2 ff ff 3f                   	mov.l	65532\[r15\], \[r15\]
 [0-9a-f]+:	e6 f0 ff 3f 3f                	mov.l	65532\[r15\], 252\[r0\]
 [0-9a-f]+:	e6 ff ff 3f 3f                	mov.l	65532\[r15\], 252\[r15\]
 [0-9a-f]+:	ea f0 ff 3f ff 3f             	mov.l	65532\[r15\], 65532\[r0\]
 [0-9a-f]+:	ea ff ff 3f ff 3f             	mov.l	65532\[r15\], 65532\[r15\]
 [0-9a-f]+:	fd 20 00                      	mov.b	r0, \[r0\+\]
 [0-9a-f]+:	fd 20 f0                      	mov.b	r0, \[r15\+\]
 [0-9a-f]+:	fd 20 0f                      	mov.b	r15, \[r0\+\]
 [0-9a-f]+:	fd 20 ff                      	mov.b	r15, \[r15\+\]
 [0-9a-f]+:	fd 21 00                      	mov.w	r0, \[r0\+\]
 [0-9a-f]+:	fd 21 f0                      	mov.w	r0, \[r15\+\]
 [0-9a-f]+:	fd 21 0f                      	mov.w	r15, \[r0\+\]
 [0-9a-f]+:	fd 21 ff                      	mov.w	r15, \[r15\+\]
 [0-9a-f]+:	fd 22 00                      	mov.l	r0, \[r0\+\]
 [0-9a-f]+:	fd 22 f0                      	mov.l	r0, \[r15\+\]
 [0-9a-f]+:	fd 22 0f                      	mov.l	r15, \[r0\+\]
 [0-9a-f]+:	fd 22 ff                      	mov.l	r15, \[r15\+\]
 [0-9a-f]+:	fd 28 00                      	mov.b	\[r0\+\], r0
 [0-9a-f]+:	fd 28 0f                      	mov.b	\[r0\+\], r15
 [0-9a-f]+:	fd 28 f0                      	mov.b	\[r15\+\], r0
 [0-9a-f]+:	fd 28 ff                      	mov.b	\[r15\+\], r15
 [0-9a-f]+:	fd 29 00                      	mov.w	\[r0\+\], r0
 [0-9a-f]+:	fd 29 0f                      	mov.w	\[r0\+\], r15
 [0-9a-f]+:	fd 29 f0                      	mov.w	\[r15\+\], r0
 [0-9a-f]+:	fd 29 ff                      	mov.w	\[r15\+\], r15
 [0-9a-f]+:	fd 2a 00                      	mov.l	\[r0\+\], r0
 [0-9a-f]+:	fd 2a 0f                      	mov.l	\[r0\+\], r15
 [0-9a-f]+:	fd 2a f0                      	mov.l	\[r15\+\], r0
 [0-9a-f]+:	fd 2a ff                      	mov.l	\[r15\+\], r15
 [0-9a-f]+:	fd 24 00                      	mov.b	r0, \[-r0\]
 [0-9a-f]+:	fd 24 f0                      	mov.b	r0, \[-r15\]
 [0-9a-f]+:	fd 24 0f                      	mov.b	r15, \[-r0\]
 [0-9a-f]+:	fd 24 ff                      	mov.b	r15, \[-r15\]
 [0-9a-f]+:	fd 25 00                      	mov.w	r0, \[-r0\]
 [0-9a-f]+:	fd 25 f0                      	mov.w	r0, \[-r15\]
 [0-9a-f]+:	fd 25 0f                      	mov.w	r15, \[-r0\]
 [0-9a-f]+:	fd 25 ff                      	mov.w	r15, \[-r15\]
 [0-9a-f]+:	fd 26 00                      	mov.l	r0, \[-r0\]
 [0-9a-f]+:	fd 26 f0                      	mov.l	r0, \[-r15\]
 [0-9a-f]+:	fd 26 0f                      	mov.l	r15, \[-r0\]
 [0-9a-f]+:	fd 26 ff                      	mov.l	r15, \[-r15\]
 [0-9a-f]+:	fd 2c 00                      	mov.b	\[-r0\], r0
 [0-9a-f]+:	fd 2c 0f                      	mov.b	\[-r0\], r15
 [0-9a-f]+:	fd 2c f0                      	mov.b	\[-r15\], r0
 [0-9a-f]+:	fd 2c ff                      	mov.b	\[-r15\], r15
 [0-9a-f]+:	fd 2d 00                      	mov.w	\[-r0\], r0
 [0-9a-f]+:	fd 2d 0f                      	mov.w	\[-r0\], r15
 [0-9a-f]+:	fd 2d f0                      	mov.w	\[-r15\], r0
 [0-9a-f]+:	fd 2d ff                      	mov.w	\[-r15\], r15
 [0-9a-f]+:	fd 2e 00                      	mov.l	\[-r0\], r0
 [0-9a-f]+:	fd 2e 0f                      	mov.l	\[-r0\], r15
 [0-9a-f]+:	fd 2e f0                      	mov.l	\[-r15\], r0
 [0-9a-f]+:	fd 2e ff                      	mov.l	\[-r15\], r15
