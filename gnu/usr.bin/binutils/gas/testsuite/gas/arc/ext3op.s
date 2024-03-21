# 3 operand insn test

	dsp_fp_div r0,r1,r2
	dsp_fp_div gp,fp,sp
	dsp_fp_div ilink,ilink,blink

	dsp_fp_div r0,r1,0
	dsp_fp_div r0,0,r2
	dsp_fp_div 0,r1,r2

	dsp_fp_div r0,r1,-1
	dsp_fp_div r0,-1,r2
	dsp_fp_div r0,r1,255
	dsp_fp_div r0,255,r2
	dsp_fp_div r0,r1,-256
	dsp_fp_div r0,-256,r2

	dsp_fp_div r1,r1,256
	dsp_fp_div r0,r1,0x3F
	dsp_fp_div r0,-257,r2

	dsp_fp_div r0,256,256

	dsp_fp_div r0,r1,foo

	dsp_fp_div.al r0,r0,r2
	dsp_fp_div.ra r3,r3,r5
	dsp_fp_div.eq r6,r6,r8
	dsp_fp_div.z  r9,r9,r11
	dsp_fp_div.ne r12,r12,r14
	dsp_fp_div.nz r15,r15,r17
	dsp_fp_div.pl r18,r18,r20
	dsp_fp_div.p  r21,r21,r23
	dsp_fp_div.mi r24,r24,r26
	dsp_fp_div.n  r27,r27,r29
	dsp_fp_div.cs r30,r30,r31
	dsp_fp_div.c  r3,r3,r3
	dsp_fp_div.lo r3,r3,r8
	dsp_fp_div.cc r3,r3,r4
	dsp_fp_div.nc r4,r4,r4
	dsp_fp_div.hs r4,r4,r7
	dsp_fp_div.vs r4,r4,r5
	dsp_fp_div.v  r5,r5,r5
	dsp_fp_div.vc r5,r5,r5
	dsp_fp_div.nv r5,r5,r5
	dsp_fp_div.gt r6,r6,r0
	dsp_fp_div.ge r0,r0,0
	dsp_fp_div.lt r1,r1,1
	dsp_fp_div.hi r3,r3,3
	dsp_fp_div.ls r4,r4,4
	dsp_fp_div.pnz r5,r5,5
	dsp_fp_div.f r0,r1,r2
	dsp_fp_div.f r0,r1,1
	dsp_fp_div.f r0,1,r2
	dsp_fp_div.f 0,r1,r2
	dsp_fp_div.f r0,r1,512
	dsp_fp_div.f r0,512,r2

	dsp_fp_div.eq.f r1,r1,r2
	dsp_fp_div.ne.f r0,r0,0
	dsp_fp_div.lt.f r2,r2,r2
	dsp_fp_div.gt.f 0,1,2
	dsp_fp_div.le.f 0,512,512
	dsp_fp_div.ge.f 0,512,2
