# Insn 3op .extInstruction test
	.extInstruction myinsn, 0x07, 0x30, SUFFIX_FLAG|SUFFIX_COND, SYNTAX_3OP

	myinsn r0,r1,r2
	myinsn r26,fp,sp
	myinsn ilink1,ilink2,blink

	myinsn r0,r1,0
	myinsn r0,0,r2
	myinsn 0,r1,r2
	myinsn r0,r1,-1
	myinsn r0,-1,r2
	myinsn r0,r1,255
	myinsn r0,255,r2
	myinsn r0,r1,-256
	myinsn r0,-256,r2

	myinsn r0,r1,256
	myinsn r0,-257,r2

	myinsn r0,256,256

	myinsn r0,r1,foo

	myinsn.al r0,r0,r2
	myinsn.ra r3,r3,r5
	myinsn.eq r6,r6,r8
	myinsn.z  r9,r9,r11
	myinsn.ne r12,r12,r14
	myinsn.nz r15,r15,r17
	myinsn.pl r18,r18,r20
	myinsn.p  r21,r21,r23
	myinsn.mi r24,r24,r26
	myinsn.n  r27,r27,r29
	myinsn.cs r30,r30,r31
	myinsn.c  r3,r3,r3
	myinsn.lo r3,r3,r8
	myinsn.cc r3,r3,r4
	myinsn.nc r4,r4,r4
	myinsn.hs r4,r4,r7
	myinsn.vs r4,r4,r5
	myinsn.v  r5,r5,r5
	myinsn.vc r5,r5,r5
	myinsn.nv r5,r5,r5
	myinsn.gt r6,r6,r0
	myinsn.ge r0,r0,0
	myinsn.lt r1,r1,1
	myinsn.hi r3,r3,3
	myinsn.ls r4,r4,4
	myinsn.pnz r5,r5,5

	myinsn.f r0,r1,r2
	myinsn.f r0,r1,1
	myinsn.f r0,1,r2
	myinsn.f 0,r1,r2
	myinsn.f r0,r1,512
	myinsn.f r0,512,r2

	myinsn.eq.f r1,r1,r2
	myinsn.ne.f r0,r0,0
	myinsn.lt.f r2,r2,r2
	myinsn.gt.f 0,1,2
	myinsn.le.f 0,512,512
	myinsn.ge.f 0,512,2
