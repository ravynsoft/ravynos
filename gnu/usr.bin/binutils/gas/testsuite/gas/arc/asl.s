# asl test

	asl r0,r1,r2
	asl r26,fp,sp
	asl ilink1,ilink2,blink

	asl r0,r1,0
	asl r0,0,r2
	asl 0,r1,r2
	asl r0,r1,-1
	asl r0,-1,r2
	asl r0,r1,255
	asl r0,255,r2
	asl r0,r1,-256
	asl r0,-256,r2

	asl r0,r1,256
	asl r0,-257,r2

	asl r0,256,256

	asl r0,r1,foo

	asl.al r0,r0,r2
	asl.ra r3,r3,r5
	asl.eq r6,r6,r8
	asl.z  r9,r9,r11
	asl.ne r12,r12,r14
	asl.nz r15,r15,r17
	asl.pl r18,r18,r20
	asl.p  r21,r21,r23
	asl.mi r24,r24,r26
	asl.n  r27,r27,r29
	asl.cs r30,r30,r31
	asl.c  r3,r3,r3
	asl.lo r3,r3,r8
	asl.cc r3,r3,r4
	asl.nc r4,r4,r4
	asl.hs r4,r4,r7
	asl.vs r4,r4,r5
	asl.v  r5,r5,r5
	asl.vc r5,r5,r5
	asl.nv r5,r5,r5
	asl.gt r6,r6,r0
	asl.ge r0,r0,0
	asl.lt r1,r1,1
	asl.hi r3,r3,3
	asl.ls r4,r4,4
	asl.pnz r5,r5,5

	asl.f r0,r1,r2
	asl.f r0,r1,1
	asl.f r0,1,r2
	asl.f 0,r1,r2
	asl.f r0,r1,512
	asl.f r0,512,r2

	asl.eq.f r1,r1,r2
	asl.ne.f r0,r0,0
	asl.lt.f r2,r2,r2
	asl.gt.f 0,1,2
	asl.le.f 0,512,512
	asl.ge.f 0,512,2
