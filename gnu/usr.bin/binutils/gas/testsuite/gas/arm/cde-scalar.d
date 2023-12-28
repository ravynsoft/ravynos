#name: Custom Datapath Extension Scalar bits (CDE)
#source: cde-scalar.s
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp7 -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp1+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7 -I$srcdir/$subdir
#objdump: -M force-thumb -dr --show-raw-insn -marmv8-m.main -M coproc0=cde -M coproc7=cde
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
