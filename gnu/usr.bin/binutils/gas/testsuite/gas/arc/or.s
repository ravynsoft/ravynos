# or test

	or r0,r1,r2
	or r26,fp,sp
	or ilink1,ilink2,blink

	or r0,r1,0
	or r0,0,r2
	or 0,r1,r2
	or r0,r1,-1
	or r0,-1,r2
	or r0,r1,255
	or r0,255,r2
	or r0,r1,-256
	or r0,-256,r2

	or r0,r1,256
	or r0,-257,r2

	or r0,256,256

	or r0,r1,foo

	or.al r0,r0,r2
	or.ra r3,r3,r5
	or.eq r6,r6,r8
	or.z  r9,r9,r11
	or.ne r12,r12,r14
	or.nz r15,r15,r17
	or.pl r18,r18,r20
	or.p  r21,r21,r23
	or.mi r24,r24,r26
	or.n  r27,r27,r29
	or.cs r30,r30,r31
	or.c  r3,r3,r3
	or.lo r3,r3,r8
	or.cc r3,r3,r4
	or.nc r4,r4,r4
	or.hs r4,r4,r7
	or.vs r4,r4,r5
	or.v  r5,r5,r5
	or.vc r5,r5,r5
	or.nv r5,r5,r5
	or.gt r6,r6,r0
	or.ge r0,r0,0
	or.lt r1,r1,1
	or.hi r3,r3,3
	or.ls r4,r4,4
	or.pnz r5,r5,5

	or.f r0,r1,r2
	or.f r0,r1,1
	or.f r0,1,r2
	or.f 0,r1,r2
	or.f r0,r1,512
	or.f r0,512,r2

	or.eq.f r1,r1,r2
	or.ne.f r0,r0,0
	or.lt.f r2,r2,r2
	or.gt.f 0,1,2
	or.le.f 0,512,512
	or.ge.f 0,512,2
