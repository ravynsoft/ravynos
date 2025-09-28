# sub test

	sub r0,r1,r2
	sub r26,fp,sp
	sub ilink1,ilink2,blink

	sub r0,r1,0
	sub r0,0,r2
	sub 0,r1,r2
	sub r0,r1,-1
	sub r0,-1,r2
	sub r0,r1,255
	sub r0,255,r2
	sub r0,r1,-256
	sub r0,-256,r2

	sub r0,r1,256
	sub r0,-257,r2

	sub r0,256,256

	sub r0,r1,foo

	sub.al r0,r0,r2
	sub.ra r3,r3,r5
	sub.eq r6,r6,r8
	sub.z  r9,r9,r11
	sub.ne r12,r12,r14
	sub.nz r15,r15,r17
	sub.pl r18,r18,r20
	sub.p  r21,r21,r23
	sub.mi r24,r24,r26
	sub.n  r27,r27,r29
	sub.cs r30,r30,r31
	sub.c  r3,r3,r3
	sub.lo r3,r3,r8
	sub.cc r3,r3,r4
	sub.nc r4,r4,r4
	sub.hs r4,r4,r7
	sub.vs r4,r4,r5
	sub.v  r5,r5,r5
	sub.vc r5,r5,r5
	sub.nv r5,r5,r5
	sub.gt r6,r6,r0
	sub.ge r0,r0,0
	sub.lt r1,r1,1
	sub.hi r3,r3,3
	sub.ls r4,r4,4
	sub.pnz r5,r5,5

	sub.f r0,r1,r2
	sub.f r0,r1,1
	sub.f r0,1,r2
	sub.f 0,r1,r2
	sub.f r0,r1,512
	sub.f r0,512,r2

	sub.eq.f r1,r1,r2
	sub.ne.f r0,r0,0
	sub.lt.f r2,r2,r2
	sub.gt.f 0,1,2
	sub.le.f 0,512,512
	sub.ge.f 0,512,2
