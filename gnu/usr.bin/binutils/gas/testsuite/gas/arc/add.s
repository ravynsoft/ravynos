# add test

	add r0,r1,r2
	add r26,fp,sp
	add ilink1,ilink2,blink

	add r0,r1,0
	add r0,0,r2
	add 0,r1,r2
	add r0,r1,-1
	add r0,-1,r2
	add r0,r1,255
	add r0,255,r2
	add r0,r1,-256
	add r0,-256,r2

	add r0,r1,256
	add r0,-257,r2

	add r0,256,256

	add r0,r1,foo

	add.al r0,r0,r2
	add.ra r3,r3,r5
	add.eq r6,r6,r8
	add.z  r9,r9,r11
	add.ne r12,r12,r14
	add.nz r15,r15,r17
	add.pl r18,r18,r20
	add.p  r21,r21,r23
	add.mi r24,r24,r26
	add.n  r27,r27,r29
	add.cs r30,r30,r31
	add.c  r3,r3,r3
	add.lo r3,r3,r8
	add.cc r3,r3,r4
	add.nc r4,r4,r4
	add.hs r4,r4,r7
	add.vs r4,r4,r5
	add.v  r5,r5,r5
	add.vc r5,r5,r5
	add.nv r5,r5,r5
	add.gt r6,r6,r0
	add.ge r0,r0,0
	add.lt r1,r1,1
	add.hi r3,r3,3
	add.ls r4,r4,4
	add.pnz r5,r5,5

	add.f r0,r1,r2
	add.f r0,r1,1
	add.f r0,1,r2
	add.f 0,r1,r2
	add.f r0,r1,512
	add.f r0,512,r2

	add.eq.f r1,r1,r2
	add.ne.f r0,r0,0
	add.lt.f r2,r2,r2
	add.gt.f 0,1,2
	add.le.f 0,512,512
	add.ge.f 0,512,2
