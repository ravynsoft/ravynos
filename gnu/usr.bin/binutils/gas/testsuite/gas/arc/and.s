# and test

	and r0,r1,r2
	and r26,fp,sp
	and ilink1,ilink2,blink

	and r0,r1,0
	and r0,0,r2
	and 0,r1,r2
	and r0,r1,-1
	and r0,-1,r2
	and r0,r1,255
	and r0,255,r2
	and r0,r1,-256
	and r0,-256,r2

	and r0,r1,256
	and r0,-257,r2

	and r0,256,256

	and r0,r1,foo

	and.al r0,r0,r2
	and.ra r3,r3,r5
	and.eq r6,r6,r8
	and.z  r9,r9,r11
	and.ne r12,r12,r14
	and.nz r15,r15,r17
	and.pl r18,r18,r20
	and.p  r21,r21,r23
	and.mi r24,r24,r26
	and.n  r27,r27,r29
	and.cs r30,r30,r31
	and.c  r3,r3,r3
	and.lo r3,r3,r8
	and.cc r3,r3,r4
	and.nc r4,r4,r4
	and.hs r4,r4,r7
	and.vs r4,r4,r5
	and.v  r5,r5,r5
	and.vc r5,r5,r5
	and.nv r5,r5,r5
	and.gt r6,r6,r0
	and.ge r0,r0,0
	and.lt r1,r1,1
	and.hi r3,r3,3
	and.ls r4,r4,4
	and.pnz r5,r5,5

	and.f r0,r1,r2
	and.f r0,r1,1
	and.f r0,1,r2
	and.f 0,r1,r2
	and.f r0,r1,512
	and.f r0,512,r2

	and.eq.f r1,r1,r2
	and.ne.f r0,r0,0
	and.lt.f r2,r2,r2
	and.gt.f 0,1,2
	and.le.f 0,512,512
	and.ge.f 0,512,2
