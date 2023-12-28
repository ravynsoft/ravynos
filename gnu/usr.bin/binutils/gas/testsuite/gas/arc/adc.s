# adc test

	adc r0,r1,r2
	adc r26,fp,sp
	adc ilink1,ilink2,blink

	adc r0,r1,0
	adc r0,0,r2
	adc 0,r1,r2
	adc r0,r1,-1
	adc r0,-1,r2
	adc r0,r1,255
	adc r0,255,r2
	adc r0,r1,-256
	adc r0,-256,r2

	adc r0,r1,256
	adc r0,-257,r2

	adc r0,256,256

	adc r0,r1,foo

	adc.al r0,r0,r2
	adc.ra r3,r3,r5
	adc.eq r6,r6,r8
	adc.z  r9,r9,r11
	adc.ne r12,r12,r14
	adc.nz r15,r15,r17
	adc.pl r18,r18,r20
	adc.p  r21,r21,r23
	adc.mi r24,r24,r26
	adc.n  r27,r27,r29
	adc.cs r30,r30,r31
	adc.c  r3,r3,r3
	adc.lo r3,r3,r8
	adc.cc r3,r3,r4
	adc.nc r4,r4,r4
	adc.hs r4,r4,r7
	adc.vs r4,r4,r5
	adc.v  r5,r5,r5
	adc.vc r5,r5,r5
	adc.nv r5,r5,r5
	adc.gt r6,r6,r0
	adc.ge r0,r0,0
	adc.lt r1,r1,1
	adc.hi r3,r3,3
	adc.ls r4,r4,4
	adc.pnz r5,r5,5

	adc.f r0,r1,r2
	adc.f r0,r1,1
	adc.f r0,1,r2
	adc.f 0,r1,r2
	adc.f r0,r1,512
	adc.f r0,512,r2

	adc.eq.f r1,r1,r2
	adc.ne.f r0,r0,0
	adc.lt.f r2,r2,r2
	adc.gt.f 0,1,2
	adc.le.f 0,512,512
	adc.ge.f 0,512,2
