# xor test

	xor r0,r1,r2
	xor r26,fp,sp
	xor ilink1,ilink2,blink

	xor r0,r1,0
	xor r0,0,r2
	xor 0,r1,r2
	xor r0,r1,-1
	xor r0,-1,r2
	xor r0,r1,255
	xor r0,255,r2
	xor r0,r1,-256
	xor r0,-256,r2

	xor r0,r1,256
	xor r0,-257,r2

	xor r0,256,256

	xor r0,r1,foo

	xor.al r0,r0,r2
	xor.ra r3,r3,r5
	xor.eq r6,r6,r8
	xor.z  r9,r9,r11
	xor.ne r12,r12,r14
	xor.nz r15,r15,r17
	xor.pl r18,r18,r20
	xor.p  r21,r21,r23
	xor.mi r24,r24,r26
	xor.n  r27,r27,r29
	xor.cs r30,r30,r31
	xor.c  r3,r3,r3
	xor.lo r3,r3,r8
	xor.cc r3,r3,r4
	xor.nc r4,r4,r4
	xor.hs r4,r4,r7
	xor.vs r4,r4,r5
	xor.v  r5,r5,r5
	xor.vc r5,r5,r5
	xor.nv r5,r5,r5
	xor.gt r6,r6,r0
	xor.ge r0,r0,0
	xor.lt r1,r1,1
	xor.hi r3,r3,3
	xor.ls r4,r4,4
	xor.pnz r5,r5,5

	xor.f r0,r1,r2
	xor.f r0,r1,1
	xor.f r0,1,r2
	xor.f 0,r1,r2
	xor.f r0,r1,512
	xor.f r0,512,r2

	xor.eq.f r1,r1,r2
	xor.ne.f r0,r0,0
	xor.lt.f r2,r2,r2
	xor.gt.f 0,1,2
	xor.le.f 0,512,512
	xor.ge.f 0,512,2
