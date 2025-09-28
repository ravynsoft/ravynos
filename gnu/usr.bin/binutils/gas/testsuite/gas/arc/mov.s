# mov test

# reg,reg
	mov r0,r1
	mov fp,sp

# shimm values
	mov r0,0
	mov r1,-1
	mov 0,r2
	mov r4,255
	mov 0,255
	mov r6,-256

# limm values
	mov r11,0x42424242
	mov 0, 0x12345678

# symbols
	mov r0,@foo

# conditional execution
	mov.al r0,r1
	mov.ra r3,r4
	mov.eq r6,r7
	mov.z  r9,r10
	mov.ne r12,r13
	mov.nz r15,r16
	mov.pl r18,r19
	mov.p  r21,r22
	mov.mi r24,r25
	mov.n  r27,r28
	mov.cs r0,r1
	mov.c  r3,r4
	mov.lo r6,r7
	mov.cc r9,r0
	mov.nc r2,r3
	mov.hs r5,r6
	mov.vs r8,r9
	mov.v  r1,r2
	mov.vc r4,r5
	mov.nv r7,r8
	mov.gt r0,r0
	mov.ge r0,0
	mov.lt 0,r1
	mov.le 0,2
	mov.hi r3,r3
	mov.ls r4,r4
	mov.pnz r5,r5

# flag setting
	mov.f r0,r1
	mov.f r2,1
	mov.f 0,r4
	mov.f r5,512

# conditional execution + flag setting
	mov.eq.f r0,r1
	mov.ne.f r1,0
	mov.lt.f 0,r2
	mov.gt.f 0,r2
	mov.le.f r0,512
	mov.ge.f 0,r2
	mov.n.f  0,512
