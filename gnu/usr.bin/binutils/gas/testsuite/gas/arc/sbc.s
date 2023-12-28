# sbc test

	sbc r0,r1,r2
	sbc r26,fp,sp
	sbc ilink1,ilink2,blink

	sbc r0,r1,0
	sbc r0,0,r2
	sbc 0,r1,r2
	sbc r0,r1,-1
	sbc r0,-1,r2
	sbc r0,r1,255
	sbc r0,255,r2
	sbc r0,r1,-256
	sbc r0,-256,r2

	sbc r0,r1,256
	sbc r0,-257,r2

	sbc r0,256,256

	sbc r0,r1,foo

	sbc.al r0,r0,r2
	sbc.ra r3,r3,r5
	sbc.eq r6,r6,r8
	sbc.z  r9,r9,r11
	sbc.ne r12,r12,r14
	sbc.nz r15,r15,r17
	sbc.pl r18,r18,r20
	sbc.p  r21,r21,r23
	sbc.mi r24,r24,r26
	sbc.n  r27,r27,r29
	sbc.cs r30,r30,r31
	sbc.c  r3,r3,r3
	sbc.lo r3,r3,r8
	sbc.cc r3,r3,r4
	sbc.nc r4,r4,r4
	sbc.hs r4,r4,r7
	sbc.vs r4,r4,r5
	sbc.v  r5,r5,r5
	sbc.vc r5,r5,r5
	sbc.nv r5,r5,r5
	sbc.gt r6,r6,r0
	sbc.ge r0,r0,0
	sbc.lt r1,r1,1
	sbc.hi r3,r3,3
	sbc.ls r4,r4,4
	sbc.pnz r5,r5,5

	sbc.f r0,r1,r2
	sbc.f r0,r1,1
	sbc.f r0,1,r2
	sbc.f 0,r1,r2
	sbc.f r0,r1,512
	sbc.f r0,512,r2

	sbc.eq.f r1,r1,r2
	sbc.ne.f r0,r0,0
	sbc.lt.f r2,r2,r2
	sbc.gt.f 0,1,2
	sbc.le.f 0,512,512
	sbc.ge.f 0,512,2
