# ror test

	ror r0,r1,r2
	ror r26,fp,sp
	ror ilink1,ilink2,blink

	ror r0,r1,0
	ror r0,0,r2
	ror 0,r1,r2
	ror r0,r1,-1
	ror r0,-1,r2
	ror r0,r1,255
	ror r0,255,r2
	ror r0,r1,-256
	ror r0,-256,r2

	ror r0,r1,256
	ror r0,-257,r2

	ror r0,256,256

	ror r0,r1,foo

	ror.al r0,r0,r2
	ror.ra r3,r3,r5
	ror.eq r6,r6,r8
	ror.z  r9,r9,r11
	ror.ne r12,r12,r14
	ror.nz r15,r15,r17
	ror.pl r18,r18,r20
	ror.p  r21,r21,r23
	ror.mi r24,r24,r26
	ror.n  r27,r27,r29
	ror.cs r30,r30,r31
	ror.c  r3,r3,r3
	ror.lo r3,r3,r8
	ror.cc r3,r3,r4
	ror.nc r4,r4,r4
	ror.hs r4,r4,r7
	ror.vs r4,r4,r5
	ror.v  r5,r5,r5
	ror.vc r5,r5,r5
	ror.nv r5,r5,r5
	ror.gt r6,r6,r0
	ror.ge r0,r0,0
	ror.lt r1,r1,1
	ror.hi r3,r3,3
	ror.ls r4,r4,4
	ror.pnz r5,r5,5

	ror.f r0,r1,r2
	ror.f r0,r1,1
	ror.f r0,1,r2
	ror.f 0,r1,r2
	ror.f r0,r1,512
	ror.f r0,512,r2

	ror.eq.f r1,r1,r2
	ror.ne.f r0,r0,0
	ror.lt.f r2,r2,r2
	ror.gt.f 0,1,2
	ror.le.f 0,512,512
	ror.ge.f 0,512,2
