#name: Custom Datapath Extension (CDE)
#source: cde.s
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+cdecp7+mve.fp -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+cdecp7+mve -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+cdecp1+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7+mve.fp -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+cdecp1+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7+mve -I$srcdir/$subdir
#objdump: -M force-thumb -dr --show-raw-insn -marmv8.1-m.main -M coproc0=cde -M coproc7=cde
#...
00000000 <\.text>:
 *[0-9a-f]+:	ee00 0000 	cx1	p0, r0, #0
 *[0-9a-f]+:	ee3f 0000 	cx1	p0, r0, #8064
 *[0-9a-f]+:	ee00 0080 	cx1	p0, r0, #64
 *[0-9a-f]+:	ee00 003f 	cx1	p0, r0, #63
 *[0-9a-f]+:	ee00 0700 	cx1	p7, r0, #0
 *[0-9a-f]+:	ee00 f000 	cx1	p0, APSR_nzcv, #0
 *[0-9a-f]+:	ee00 9000 	cx1	p0, r9, #0
 *[0-9a-f]+:	fe00 0000 	cx1a	p0, r0, #0
 *[0-9a-f]+:	fe3f 0000 	cx1a	p0, r0, #8064
 *[0-9a-f]+:	fe00 0080 	cx1a	p0, r0, #64
 *[0-9a-f]+:	fe00 003f 	cx1a	p0, r0, #63
 *[0-9a-f]+:	fe00 0700 	cx1a	p7, r0, #0
 *[0-9a-f]+:	fe00 f000 	cx1a	p0, APSR_nzcv, #0
 *[0-9a-f]+:	fe00 9000 	cx1a	p0, r9, #0
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	fe00 0000 	cx1a	p0, r0, #0
 *[0-9a-f]+:	ee00 0040 	cx1d	p0, r0, r1, #0
 *[0-9a-f]+:	ee3f 0040 	cx1d	p0, r0, r1, #8064
 *[0-9a-f]+:	ee00 00c0 	cx1d	p0, r0, r1, #64
 *[0-9a-f]+:	ee00 007f 	cx1d	p0, r0, r1, #63
 *[0-9a-f]+:	ee00 0740 	cx1d	p7, r0, r1, #0
 *[0-9a-f]+:	ee00 a040 	cx1d	p0, sl, fp, #0
 *[0-9a-f]+:	fe00 0040 	cx1da	p0, r0, r1, #0
 *[0-9a-f]+:	fe3f 0040 	cx1da	p0, r0, r1, #8064
 *[0-9a-f]+:	fe00 00c0 	cx1da	p0, r0, r1, #64
 *[0-9a-f]+:	fe00 007f 	cx1da	p0, r0, r1, #63
 *[0-9a-f]+:	fe00 0740 	cx1da	p7, r0, r1, #0
 *[0-9a-f]+:	fe00 a040 	cx1da	p0, sl, fp, #0
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	fe00 0040 	cx1da	p0, r0, r1, #0
 *[0-9a-f]+:	ee40 0000 	cx2	p0, r0, r0, #0
 *[0-9a-f]+:	ee70 0000 	cx2	p0, r0, r0, #384
 *[0-9a-f]+:	ee40 0080 	cx2	p0, r0, r0, #64
 *[0-9a-f]+:	ee40 003f 	cx2	p0, r0, r0, #63
 *[0-9a-f]+:	ee40 0700 	cx2	p7, r0, r0, #0
 *[0-9a-f]+:	ee40 f000 	cx2	p0, APSR_nzcv, r0, #0
 *[0-9a-f]+:	ee40 9000 	cx2	p0, r9, r0, #0
 *[0-9a-f]+:	ee4f 0000 	cx2	p0, r0, APSR_nzcv, #0
 *[0-9a-f]+:	ee49 0000 	cx2	p0, r0, r9, #0
 *[0-9a-f]+:	fe40 0000 	cx2a	p0, r0, r0, #0
 *[0-9a-f]+:	fe70 0000 	cx2a	p0, r0, r0, #384
 *[0-9a-f]+:	fe40 0080 	cx2a	p0, r0, r0, #64
 *[0-9a-f]+:	fe40 003f 	cx2a	p0, r0, r0, #63
 *[0-9a-f]+:	fe40 0700 	cx2a	p7, r0, r0, #0
 *[0-9a-f]+:	fe40 f000 	cx2a	p0, APSR_nzcv, r0, #0
 *[0-9a-f]+:	fe40 9000 	cx2a	p0, r9, r0, #0
 *[0-9a-f]+:	fe4f 0000 	cx2a	p0, r0, APSR_nzcv, #0
 *[0-9a-f]+:	fe49 0000 	cx2a	p0, r0, r9, #0
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	fe40 0000 	cx2a	p0, r0, r0, #0
 *[0-9a-f]+:	ee40 0040 	cx2d	p0, r0, r1, r0, #0
 *[0-9a-f]+:	ee70 0040 	cx2d	p0, r0, r1, r0, #384
 *[0-9a-f]+:	ee40 00c0 	cx2d	p0, r0, r1, r0, #64
 *[0-9a-f]+:	ee40 007f 	cx2d	p0, r0, r1, r0, #63
 *[0-9a-f]+:	ee40 0740 	cx2d	p7, r0, r1, r0, #0
 *[0-9a-f]+:	ee40 a040 	cx2d	p0, sl, fp, r0, #0
 *[0-9a-f]+:	ee4f 0040 	cx2d	p0, r0, r1, APSR_nzcv, #0
 *[0-9a-f]+:	ee49 0040 	cx2d	p0, r0, r1, r9, #0
 *[0-9a-f]+:	fe40 0040 	cx2da	p0, r0, r1, r0, #0
 *[0-9a-f]+:	fe70 0040 	cx2da	p0, r0, r1, r0, #384
 *[0-9a-f]+:	fe40 00c0 	cx2da	p0, r0, r1, r0, #64
 *[0-9a-f]+:	fe40 007f 	cx2da	p0, r0, r1, r0, #63
 *[0-9a-f]+:	fe40 0740 	cx2da	p7, r0, r1, r0, #0
 *[0-9a-f]+:	fe40 a040 	cx2da	p0, sl, fp, r0, #0
 *[0-9a-f]+:	fe4f 0040 	cx2da	p0, r0, r1, APSR_nzcv, #0
 *[0-9a-f]+:	fe49 0040 	cx2da	p0, r0, r1, r9, #0
 *[0-9a-f]+:	ee80 0000 	cx3	p0, r0, r0, r0, #0
 *[0-9a-f]+:	eef0 0000 	cx3	p0, r0, r0, r0, #56
 *[0-9a-f]+:	ee80 0080 	cx3	p0, r0, r0, r0, #4
 *[0-9a-f]+:	ee80 0030 	cx3	p0, r0, r0, r0, #3
 *[0-9a-f]+:	ee80 0700 	cx3	p7, r0, r0, r0, #0
 *[0-9a-f]+:	ee80 000f 	cx3	p0, APSR_nzcv, r0, r0, #0
 *[0-9a-f]+:	ee80 0009 	cx3	p0, r9, r0, r0, #0
 *[0-9a-f]+:	ee8f 0000 	cx3	p0, r0, APSR_nzcv, r0, #0
 *[0-9a-f]+:	ee89 0000 	cx3	p0, r0, r9, r0, #0
 *[0-9a-f]+:	ee80 f000 	cx3	p0, r0, r0, APSR_nzcv, #0
 *[0-9a-f]+:	ee80 9000 	cx3	p0, r0, r0, r9, #0
 *[0-9a-f]+:	fe80 0000 	cx3a	p0, r0, r0, r0, #0
 *[0-9a-f]+:	fef0 0000 	cx3a	p0, r0, r0, r0, #56
 *[0-9a-f]+:	fe80 0080 	cx3a	p0, r0, r0, r0, #4
 *[0-9a-f]+:	fe80 0030 	cx3a	p0, r0, r0, r0, #3
 *[0-9a-f]+:	fe80 0700 	cx3a	p7, r0, r0, r0, #0
 *[0-9a-f]+:	fe80 000f 	cx3a	p0, APSR_nzcv, r0, r0, #0
 *[0-9a-f]+:	fe80 0009 	cx3a	p0, r9, r0, r0, #0
 *[0-9a-f]+:	fe8f 0000 	cx3a	p0, r0, APSR_nzcv, r0, #0
 *[0-9a-f]+:	fe89 0000 	cx3a	p0, r0, r9, r0, #0
 *[0-9a-f]+:	fe80 f000 	cx3a	p0, r0, r0, APSR_nzcv, #0
 *[0-9a-f]+:	fe80 9000 	cx3a	p0, r0, r0, r9, #0
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	fe80 0000 	cx3a	p0, r0, r0, r0, #0
 *[0-9a-f]+:	ee80 0040 	cx3d	p0, r0, r1, r0, r0, #0
 *[0-9a-f]+:	eef0 0040 	cx3d	p0, r0, r1, r0, r0, #56
 *[0-9a-f]+:	ee80 00c0 	cx3d	p0, r0, r1, r0, r0, #4
 *[0-9a-f]+:	ee80 0070 	cx3d	p0, r0, r1, r0, r0, #3
 *[0-9a-f]+:	ee80 0740 	cx3d	p7, r0, r1, r0, r0, #0
 *[0-9a-f]+:	ee80 004a 	cx3d	p0, sl, fp, r0, r0, #0
 *[0-9a-f]+:	ee8f 0040 	cx3d	p0, r0, r1, APSR_nzcv, r0, #0
 *[0-9a-f]+:	ee89 0040 	cx3d	p0, r0, r1, r9, r0, #0
 *[0-9a-f]+:	ee80 f040 	cx3d	p0, r0, r1, r0, APSR_nzcv, #0
 *[0-9a-f]+:	ee80 9040 	cx3d	p0, r0, r1, r0, r9, #0
 *[0-9a-f]+:	fe80 0040 	cx3da	p0, r0, r1, r0, r0, #0
 *[0-9a-f]+:	fef0 0040 	cx3da	p0, r0, r1, r0, r0, #56
 *[0-9a-f]+:	fe80 00c0 	cx3da	p0, r0, r1, r0, r0, #4
 *[0-9a-f]+:	fe80 0070 	cx3da	p0, r0, r1, r0, r0, #3
 *[0-9a-f]+:	fe80 0740 	cx3da	p7, r0, r1, r0, r0, #0
 *[0-9a-f]+:	fe80 004a 	cx3da	p0, sl, fp, r0, r0, #0
 *[0-9a-f]+:	fe8f 0040 	cx3da	p0, r0, r1, APSR_nzcv, r0, #0
 *[0-9a-f]+:	fe89 0040 	cx3da	p0, r0, r1, r9, r0, #0
 *[0-9a-f]+:	fe80 f040 	cx3da	p0, r0, r1, r0, APSR_nzcv, #0
 *[0-9a-f]+:	fe80 9040 	cx3da	p0, r0, r1, r0, r9, #0
 *[0-9a-f]+:	ec20 0040 	vcx1	p0, q0, #0
 *[0-9a-f]+:	ed20 0040 	vcx1	p0, q0, #2048
 *[0-9a-f]+:	ec2f 0040 	vcx1	p0, q0, #1920
 *[0-9a-f]+:	ec20 00c0 	vcx1	p0, q0, #64
 *[0-9a-f]+:	ec20 007f 	vcx1	p0, q0, #63
 *[0-9a-f]+:	ec20 0740 	vcx1	p7, q0, #0
 *[0-9a-f]+:	ec20 e040 	vcx1	p0, q7, #0
 *[0-9a-f]+:	fc20 0040 	vcx1a	p0, q0, #0
 *[0-9a-f]+:	fd20 0040 	vcx1a	p0, q0, #2048
 *[0-9a-f]+:	fc2f 0040 	vcx1a	p0, q0, #1920
 *[0-9a-f]+:	fc20 00c0 	vcx1a	p0, q0, #64
 *[0-9a-f]+:	fc20 007f 	vcx1a	p0, q0, #63
 *[0-9a-f]+:	fc20 0740 	vcx1a	p7, q0, #0
 *[0-9a-f]+:	fc20 e040 	vcx1a	p0, q7, #0
 *[0-9a-f]+:	fe01 8f00 	vptt\.i8	eq, q0, q0
 *[0-9a-f]+:	ec20 0040 	vcx1	p0, q0, #0
 *[0-9a-f]+:	fc20 0040 	vcx1a	p0, q0, #0
 *[0-9a-f]+:	ec30 0040 	vcx2	p0, q0, q0, #0
 *[0-9a-f]+:	ed30 0040 	vcx2	p0, q0, q0, #64
 *[0-9a-f]+:	ec3f 0040 	vcx2	p0, q0, q0, #60
 *[0-9a-f]+:	ec30 00c0 	vcx2	p0, q0, q0, #2
 *[0-9a-f]+:	ec30 0050 	vcx2	p0, q0, q0, #1
 *[0-9a-f]+:	ec30 0740 	vcx2	p7, q0, q0, #0
 *[0-9a-f]+:	ec30 e040 	vcx2	p0, q7, q0, #0
 *[0-9a-f]+:	ec30 004e 	vcx2	p0, q0, q7, #0
 *[0-9a-f]+:	fc30 0040 	vcx2a	p0, q0, q0, #0
 *[0-9a-f]+:	fd30 0040 	vcx2a	p0, q0, q0, #64
 *[0-9a-f]+:	fc3f 0040 	vcx2a	p0, q0, q0, #60
 *[0-9a-f]+:	fc30 00c0 	vcx2a	p0, q0, q0, #2
 *[0-9a-f]+:	fc30 0050 	vcx2a	p0, q0, q0, #1
 *[0-9a-f]+:	fc30 0740 	vcx2a	p7, q0, q0, #0
 *[0-9a-f]+:	fc30 e040 	vcx2a	p0, q7, q0, #0
 *[0-9a-f]+:	fc30 004e 	vcx2a	p0, q0, q7, #0
 *[0-9a-f]+:	fe01 8f00 	vptt\.i8	eq, q0, q0
 *[0-9a-f]+:	ec30 0040 	vcx2	p0, q0, q0, #0
 *[0-9a-f]+:	fc30 0040 	vcx2a	p0, q0, q0, #0
 *[0-9a-f]+:	ec80 0040 	vcx3	p0, q0, q0, q0, #0
 *[0-9a-f]+:	ed80 0040 	vcx3	p0, q0, q0, q0, #8
 *[0-9a-f]+:	ecb0 0040 	vcx3	p0, q0, q0, q0, #6
 *[0-9a-f]+:	ec80 0050 	vcx3	p0, q0, q0, q0, #1
 *[0-9a-f]+:	ec80 0740 	vcx3	p7, q0, q0, q0, #0
 *[0-9a-f]+:	ec80 e040 	vcx3	p0, q7, q0, q0, #0
 *[0-9a-f]+:	ec8e 0040 	vcx3	p0, q0, q7, q0, #0
 *[0-9a-f]+:	ec80 004e 	vcx3	p0, q0, q0, q7, #0
 *[0-9a-f]+:	fc80 0040 	vcx3a	p0, q0, q0, q0, #0
 *[0-9a-f]+:	fd80 0040 	vcx3a	p0, q0, q0, q0, #8
 *[0-9a-f]+:	fcb0 0040 	vcx3a	p0, q0, q0, q0, #6
 *[0-9a-f]+:	fc80 0050 	vcx3a	p0, q0, q0, q0, #1
 *[0-9a-f]+:	fc80 0740 	vcx3a	p7, q0, q0, q0, #0
 *[0-9a-f]+:	fc80 e040 	vcx3a	p0, q7, q0, q0, #0
 *[0-9a-f]+:	fc8e 0040 	vcx3a	p0, q0, q7, q0, #0
 *[0-9a-f]+:	fc80 004e 	vcx3a	p0, q0, q0, q7, #0
 *[0-9a-f]+:	fe01 8f00 	vptt\.i8	eq, q0, q0
 *[0-9a-f]+:	ec80 0040 	vcx3	p0, q0, q0, q0, #0
 *[0-9a-f]+:	fc80 0040 	vcx3a	p0, q0, q0, q0, #0
 *[0-9a-f]+:	ec20 0000 	vcx1	p0, s0, #0
 *[0-9a-f]+:	ec2f 0000 	vcx1	p0, s0, #1920
 *[0-9a-f]+:	ec20 0080 	vcx1	p0, s0, #64
 *[0-9a-f]+:	ec20 003f 	vcx1	p0, s0, #63
 *[0-9a-f]+:	ec20 0700 	vcx1	p7, s0, #0
 *[0-9a-f]+:	ec60 0000 	vcx1	p0, s1, #0
 *[0-9a-f]+:	ec20 f000 	vcx1	p0, s30, #0
 *[0-9a-f]+:	ed20 0000 	vcx1	p0, d0, #0
 *[0-9a-f]+:	ed2f 0000 	vcx1	p0, d0, #1920
 *[0-9a-f]+:	ed20 0080 	vcx1	p0, d0, #64
 *[0-9a-f]+:	ed20 003f 	vcx1	p0, d0, #63
 *[0-9a-f]+:	ed20 0700 	vcx1	p7, d0, #0
 *[0-9a-f]+:	ed20 f000 	vcx1	p0, d15, #0
 *[0-9a-f]+:	fc20 0000 	vcx1a	p0, s0, #0
 *[0-9a-f]+:	fc2f 0000 	vcx1a	p0, s0, #1920
 *[0-9a-f]+:	fc20 0080 	vcx1a	p0, s0, #64
 *[0-9a-f]+:	fc20 003f 	vcx1a	p0, s0, #63
 *[0-9a-f]+:	fc20 0700 	vcx1a	p7, s0, #0
 *[0-9a-f]+:	fc60 0000 	vcx1a	p0, s1, #0
 *[0-9a-f]+:	fc20 f000 	vcx1a	p0, s30, #0
 *[0-9a-f]+:	fd20 0000 	vcx1a	p0, d0, #0
 *[0-9a-f]+:	fd2f 0000 	vcx1a	p0, d0, #1920
 *[0-9a-f]+:	fd20 0080 	vcx1a	p0, d0, #64
 *[0-9a-f]+:	fd20 003f 	vcx1a	p0, d0, #63
 *[0-9a-f]+:	fd20 0700 	vcx1a	p7, d0, #0
 *[0-9a-f]+:	fd20 f000 	vcx1a	p0, d15, #0
 *[0-9a-f]+:	ec30 0000 	vcx2	p0, s0, s0, #0
 *[0-9a-f]+:	ec3f 0000 	vcx2	p0, s0, s0, #60
 *[0-9a-f]+:	ec30 0080 	vcx2	p0, s0, s0, #2
 *[0-9a-f]+:	ec30 0010 	vcx2	p0, s0, s0, #1
 *[0-9a-f]+:	ec30 0700 	vcx2	p7, s0, s0, #0
 *[0-9a-f]+:	ec70 0000 	vcx2	p0, s1, s0, #0
 *[0-9a-f]+:	ec30 f000 	vcx2	p0, s30, s0, #0
 *[0-9a-f]+:	ec30 0020 	vcx2	p0, s0, s1, #0
 *[0-9a-f]+:	ec30 000f 	vcx2	p0, s0, s30, #0
 *[0-9a-f]+:	ed30 0000 	vcx2	p0, d0, d0, #0
 *[0-9a-f]+:	ed3f 0000 	vcx2	p0, d0, d0, #60
 *[0-9a-f]+:	ed30 0080 	vcx2	p0, d0, d0, #2
 *[0-9a-f]+:	ed30 0010 	vcx2	p0, d0, d0, #1
 *[0-9a-f]+:	ed30 0700 	vcx2	p7, d0, d0, #0
 *[0-9a-f]+:	ed30 f000 	vcx2	p0, d15, d0, #0
 *[0-9a-f]+:	ed30 000f 	vcx2	p0, d0, d15, #0
 *[0-9a-f]+:	fc30 0000 	vcx2a	p0, s0, s0, #0
 *[0-9a-f]+:	fc3f 0000 	vcx2a	p0, s0, s0, #60
 *[0-9a-f]+:	fc30 0080 	vcx2a	p0, s0, s0, #2
 *[0-9a-f]+:	fc30 0010 	vcx2a	p0, s0, s0, #1
 *[0-9a-f]+:	fc30 0700 	vcx2a	p7, s0, s0, #0
 *[0-9a-f]+:	fc70 0000 	vcx2a	p0, s1, s0, #0
 *[0-9a-f]+:	fc30 f000 	vcx2a	p0, s30, s0, #0
 *[0-9a-f]+:	fc30 0020 	vcx2a	p0, s0, s1, #0
 *[0-9a-f]+:	fc30 000f 	vcx2a	p0, s0, s30, #0
 *[0-9a-f]+:	fd30 0000 	vcx2a	p0, d0, d0, #0
 *[0-9a-f]+:	fd3f 0000 	vcx2a	p0, d0, d0, #60
 *[0-9a-f]+:	fd30 0080 	vcx2a	p0, d0, d0, #2
 *[0-9a-f]+:	fd30 0010 	vcx2a	p0, d0, d0, #1
 *[0-9a-f]+:	fd30 0700 	vcx2a	p7, d0, d0, #0
 *[0-9a-f]+:	fd30 f000 	vcx2a	p0, d15, d0, #0
 *[0-9a-f]+:	fd30 000f 	vcx2a	p0, d0, d15, #0
 *[0-9a-f]+:	ec80 0000 	vcx3	p0, s0, s0, s0, #0
 *[0-9a-f]+:	ecb0 0000 	vcx3	p0, s0, s0, s0, #6
 *[0-9a-f]+:	ec80 0010 	vcx3	p0, s0, s0, s0, #1
 *[0-9a-f]+:	ec80 0700 	vcx3	p7, s0, s0, s0, #0
 *[0-9a-f]+:	ecc0 0000 	vcx3	p0, s1, s0, s0, #0
 *[0-9a-f]+:	ec80 f000 	vcx3	p0, s30, s0, s0, #0
 *[0-9a-f]+:	ec80 0080 	vcx3	p0, s0, s1, s0, #0
 *[0-9a-f]+:	ec8f 0000 	vcx3	p0, s0, s30, s0, #0
 *[0-9a-f]+:	ec80 0020 	vcx3	p0, s0, s0, s1, #0
 *[0-9a-f]+:	ec80 000f 	vcx3	p0, s0, s0, s30, #0
 *[0-9a-f]+:	ed80 0000 	vcx3	p0, d0, d0, d0, #0
 *[0-9a-f]+:	edb0 0000 	vcx3	p0, d0, d0, d0, #6
 *[0-9a-f]+:	ed80 0010 	vcx3	p0, d0, d0, d0, #1
 *[0-9a-f]+:	ed80 0700 	vcx3	p7, d0, d0, d0, #0
 *[0-9a-f]+:	ed80 f000 	vcx3	p0, d15, d0, d0, #0
 *[0-9a-f]+:	ed8f 0000 	vcx3	p0, d0, d15, d0, #0
 *[0-9a-f]+:	ed80 000f 	vcx3	p0, d0, d0, d15, #0
 *[0-9a-f]+:	fc80 0000 	vcx3a	p0, s0, s0, s0, #0
 *[0-9a-f]+:	fcb0 0000 	vcx3a	p0, s0, s0, s0, #6
 *[0-9a-f]+:	fc80 0010 	vcx3a	p0, s0, s0, s0, #1
 *[0-9a-f]+:	fc80 0700 	vcx3a	p7, s0, s0, s0, #0
 *[0-9a-f]+:	fcc0 0000 	vcx3a	p0, s1, s0, s0, #0
 *[0-9a-f]+:	fc80 f000 	vcx3a	p0, s30, s0, s0, #0
 *[0-9a-f]+:	fc80 0080 	vcx3a	p0, s0, s1, s0, #0
 *[0-9a-f]+:	fc8f 0000 	vcx3a	p0, s0, s30, s0, #0
 *[0-9a-f]+:	fc80 0020 	vcx3a	p0, s0, s0, s1, #0
 *[0-9a-f]+:	fc80 000f 	vcx3a	p0, s0, s0, s30, #0
 *[0-9a-f]+:	fd80 0000 	vcx3a	p0, d0, d0, d0, #0
 *[0-9a-f]+:	fdb0 0000 	vcx3a	p0, d0, d0, d0, #6
 *[0-9a-f]+:	fd80 0010 	vcx3a	p0, d0, d0, d0, #1
 *[0-9a-f]+:	fd80 0700 	vcx3a	p7, d0, d0, d0, #0
 *[0-9a-f]+:	fd80 f000 	vcx3a	p0, d15, d0, d0, #0
 *[0-9a-f]+:	fd8f 0000 	vcx3a	p0, d0, d15, d0, #0
 *[0-9a-f]+:	fd80 000f 	vcx3a	p0, d0, d0, d15, #0
