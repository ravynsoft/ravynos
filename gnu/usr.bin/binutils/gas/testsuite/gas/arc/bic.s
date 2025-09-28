# bic test

	bic r0,r1,r2
	bic r26,fp,sp
	bic ilink1,ilink2,blink

	bic r0,r1,0
	bic r0,0,r2
	bic 0,r1,r2
	bic r0,r1,-1
	bic r0,-1,r2
	bic r0,r1,255
	bic r0,255,r2
	bic r0,r1,-256
	bic r0,-256,r2

	bic r0,r1,256
	bic r0,-257,r2

	bic r0,256,256

	bic r0,r1,foo

	bic.al r0,r0,r2
	bic.ra r3,r3,r5
	bic.eq r6,r6,r8
	bic.z  r9,r9,r11
	bic.ne r12,r12,r14
	bic.nz r15,r15,r17
	bic.pl r18,r18,r20
	bic.p  r21,r21,r23
	bic.mi r24,r24,r26
	bic.n  r27,r27,r29
	bic.cs r30,r30,r31
	bic.c  r3,r3,r3
	bic.lo r3,r3,r8
	bic.cc r3,r3,r4
	bic.nc r4,r4,r4
	bic.hs r4,r4,r7
	bic.vs r4,r4,r5
	bic.v  r5,r5,r5
	bic.vc r5,r5,r5
	bic.nv r5,r5,r5
	bic.gt r6,r6,r0
	bic.ge r0,r0,0
	bic.lt r1,r1,1
	bic.hi r3,r3,3
	bic.ls r4,r4,4
	bic.pnz r5,r5,5

	bic.f r0,r1,r2
	bic.f r0,r1,1
	bic.f r0,1,r2
	bic.f 0,r1,r2
	bic.f r0,r1,512
	bic.f r0,512,r2

	bic.eq.f r1,r1,r2
	bic.ne.f r0,r0,0
	bic.lt.f r2,r2,r2
	bic.gt.f 0,1,2
	bic.le.f 0,512,512
	bic.ge.f 0,512,2
