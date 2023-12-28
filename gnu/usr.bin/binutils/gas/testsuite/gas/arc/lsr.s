# lsr test

	lsr r0,r1,r2
	lsr r26,fp,sp
	lsr ilink1,ilink2,blink

	lsr r0,r1,0
	lsr r0,0,r2
	lsr 0,r1,r2
	lsr r0,r1,-1
	lsr r0,-1,r2
	lsr r0,r1,255
	lsr r0,255,r2
	lsr r0,r1,-256
	lsr r0,-256,r2

	lsr r0,r1,256
	lsr r0,-257,r2

	lsr r0,256,256

	lsr r0,r1,foo

	lsr.al r0,r0,r2
	lsr.ra r3,r3,r5
	lsr.eq r6,r6,r8
	lsr.z  r9,r9,r11
	lsr.ne r12,r12,r14
	lsr.nz r15,r15,r17
	lsr.pl r18,r18,r20
	lsr.p  r21,r21,r23
	lsr.mi r24,r24,r26
	lsr.n  r27,r27,r29
	lsr.cs r30,r30,r31
	lsr.c  r3,r3,r3
	lsr.lo r3,r3,r8
	lsr.cc r3,r3,r4
	lsr.nc r4,r4,r4
	lsr.hs r4,r4,r7
	lsr.vs r4,r4,r5
	lsr.v  r5,r5,r5
	lsr.vc r5,r5,r5
	lsr.nv r5,r5,r5
	lsr.gt r6,r6,r0
	lsr.ge r0,r0,0
	lsr.lt r1,r1,1
	lsr.hi r3,r3,3
	lsr.ls r4,r4,4
	lsr.pnz r5,r5,5

	lsr.f r0,r1,r2
	lsr.f r0,r1,1
	lsr.f r0,1,r2
	lsr.f 0,r1,r2
	lsr.f r0,r1,512
	lsr.f r0,512,r2

	lsr.eq.f r1,r1,r2
	lsr.ne.f r0,r0,0
	lsr.lt.f r2,r2,r2
	lsr.gt.f 0,1,2
	lsr.le.f 0,512,512
	lsr.ge.f 0,512,2
