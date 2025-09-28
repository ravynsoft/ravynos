# asr test

	asr r0,r1,r2
	asr r26,fp,sp
	asr ilink1,ilink2,blink

	asr r0,r1,0
	asr r0,0,r2
	asr 0,r1,r2
	asr r0,r1,-1
	asr r0,-1,r2
	asr r0,r1,255
	asr r0,255,r2
	asr r0,r1,-256
	asr r0,-256,r2

	asr r0,r1,256
	asr r0,-257,r2

	asr r0,256,256

	asr r0,r1,foo

	asr.al r0,r0,r2
	asr.ra r3,r3,r5
	asr.eq r6,r6,r8
	asr.z  r9,r9,r11
	asr.ne r12,r12,r14
	asr.nz r15,r15,r17
	asr.pl r18,r18,r20
	asr.p  r21,r21,r23
	asr.mi r24,r24,r26
	asr.n  r27,r27,r29
	asr.cs r30,r30,r31
	asr.c  r3,r3,r3
	asr.lo r3,r3,r8
	asr.cc r3,r3,r4
	asr.nc r4,r4,r4
	asr.hs r4,r4,r7
	asr.vs r4,r4,r5
	asr.v  r5,r5,r5
	asr.vc r5,r5,r5
	asr.nv r5,r5,r5
	asr.gt r6,r6,r0
	asr.ge r0,r0,0
	asr.lt r1,r1,1
	asr.hi r3,r3,3
	asr.ls r4,r4,4
	asr.pnz r5,r5,5

	asr.f r0,r1,r2
	asr.f r0,r1,1
	asr.f r0,1,r2
	asr.f 0,r1,r2
	asr.f r0,r1,512
	asr.f r0,512,r2

	asr.eq.f r1,r1,r2
	asr.ne.f r0,r0,0
	asr.lt.f r2,r2,r2
	asr.gt.f 0,1,2
	asr.le.f 0,512,512
	asr.ge.f 0,512,2
