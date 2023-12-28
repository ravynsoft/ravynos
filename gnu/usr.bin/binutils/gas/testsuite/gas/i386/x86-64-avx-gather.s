# Check 64bit AVX gather instructions

	.text
_start:
	vgatherdpd %xmm2, (%rbp, %xmm7, 2),%xmm1
	vgatherqpd %xmm2, (%rbp, %xmm7, 2),%xmm1
	vgatherdpd %ymm2, (%rbp, %xmm7, 2),%ymm1
	vgatherqpd %ymm2, (%rbp, %ymm7, 2),%ymm1

	vgatherdpd %xmm12, (%r13, %xmm14, 2),%xmm11
	vgatherqpd %xmm12, (%r13, %xmm14, 2),%xmm11
	vgatherdpd %ymm12, (%r13, %xmm14, 2),%ymm11
	vgatherqpd %ymm12, (%r13, %ymm14, 2),%ymm11

	vgatherdpd %ymm5,0x8(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,0x8(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm4,8),%ymm6

	vgatherdpd %ymm5,0x8(,%xmm14,1),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm14,1),%ymm6
	vgatherdpd %ymm5,(,%xmm14,1),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm14,1),%ymm6
	vgatherdpd %ymm5,0x8(,%xmm14,8),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm14,8),%ymm6
	vgatherdpd %ymm5,(,%xmm14,8),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm14,8),%ymm6

	vgatherdps %xmm2, (%rbp, %xmm7, 2),%xmm1
	vgatherqps %xmm2, (%rbp, %xmm7, 2),%xmm1
	vgatherdps %ymm2, (%rbp, %ymm7, 2),%ymm1
	vgatherqps %xmm2, (%rbp, %ymm7, 2),%xmm1

	vgatherdps %xmm12, (%r13, %xmm14, 2),%xmm11
	vgatherqps %xmm12, (%r13, %xmm14, 2),%xmm11
	vgatherdps %ymm12, (%r13, %ymm14, 2),%ymm11
	vgatherqps %xmm12, (%r13, %ymm14, 2),%xmm11

	vgatherdps %xmm5,0x8(,%xmm4,1),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm4,1),%xmm6
	vgatherdps %xmm5,(,%xmm4,1),%xmm6
	vgatherdps %xmm5,0x298(,%xmm4,1),%xmm6
	vgatherdps %xmm5,0x8(,%xmm4,8),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm4,8),%xmm6
	vgatherdps %xmm5,(,%xmm4,8),%xmm6
	vgatherdps %xmm5,0x298(,%xmm4,8),%xmm6

	vgatherdps %xmm5,0x8(,%xmm14,1),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm14,1),%xmm6
	vgatherdps %xmm5,(,%xmm14,1),%xmm6
	vgatherdps %xmm5,0x298(,%xmm14,1),%xmm6
	vgatherdps %xmm5,0x8(,%xmm14,8),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm14,8),%xmm6
	vgatherdps %xmm5,(,%xmm14,8),%xmm6
	vgatherdps %xmm5,0x298(,%xmm14,8),%xmm6

	vpgatherdd %xmm2, (%rbp, %xmm7, 2),%xmm1
	vpgatherqd %xmm2, (%rbp, %xmm7, 2),%xmm1
	vpgatherdd %ymm2, (%rbp, %ymm7, 2),%ymm1
	vpgatherqd %xmm2, (%rbp, %ymm7, 2),%xmm1

	vpgatherdd %xmm12, (%r13, %xmm14, 2),%xmm11
	vpgatherqd %xmm12, (%r13, %xmm14, 2),%xmm11
	vpgatherdd %ymm12, (%r13, %ymm14, 2),%ymm11
	vpgatherqd %xmm12, (%r13, %ymm14, 2),%xmm11

	vpgatherdd %xmm5,0x8(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,0x8(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm4,8),%xmm6

	vpgatherdd %xmm5,0x8(,%xmm14,1),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm14,1),%xmm6
	vpgatherdd %xmm5,(,%xmm14,1),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm14,1),%xmm6
	vpgatherdd %xmm5,0x8(,%xmm14,8),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm14,8),%xmm6
	vpgatherdd %xmm5,(,%xmm14,8),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm14,8),%xmm6

	vpgatherdq %xmm2, (%rbp, %xmm7, 2),%xmm1
	vpgatherqq %xmm2, (%rbp, %xmm7, 2),%xmm1
	vpgatherdq %ymm2, (%rbp, %xmm7, 2),%ymm1
	vpgatherqq %ymm2, (%rbp, %ymm7, 2),%ymm1

	vpgatherdq %xmm12, (%r13, %xmm14, 2),%xmm11
	vpgatherqq %xmm12, (%r13, %xmm14, 2),%xmm11
	vpgatherdq %ymm12, (%r13, %xmm14, 2),%ymm11
	vpgatherqq %ymm12, (%r13, %ymm14, 2),%ymm11

	vpgatherdq %ymm5,0x8(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,0x8(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm4,8),%ymm6

	vpgatherdq %ymm5,0x8(,%xmm14,1),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm14,1),%ymm6
	vpgatherdq %ymm5,(,%xmm14,1),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm14,1),%ymm6
	vpgatherdq %ymm5,0x8(,%xmm14,8),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm14,8),%ymm6
	vpgatherdq %ymm5,(,%xmm14,8),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm14,8),%ymm6

	.intel_syntax noprefix
vgatherdpd xmm1,QWORD PTR [rbp+xmm7*2+0x0],xmm2
vgatherqpd xmm1,QWORD PTR [rbp+xmm7*2+0x0],xmm2
vgatherdpd ymm1,QWORD PTR [rbp+xmm7*2+0x0],ymm2
vgatherqpd ymm1,QWORD PTR [rbp+ymm7*2+0x0],ymm2
vgatherdpd xmm11,QWORD PTR [r13+xmm14*2+0x0],xmm12
vgatherqpd xmm11,QWORD PTR [r13+xmm14*2+0x0],xmm12
vgatherdpd ymm11,QWORD PTR [r13+xmm14*2+0x0],ymm12
vgatherqpd ymm11,QWORD PTR [r13+ymm14*2+0x0],ymm12
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x298],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x298],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*1+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*1-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*1+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*1+0x298],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*8+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*8-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*8+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm14*8+0x298],ymm5
vgatherdps xmm1,DWORD PTR [rbp+xmm7*2+0x0],xmm2
vgatherqps xmm1,DWORD PTR [rbp+xmm7*2+0x0],xmm2
vgatherdps ymm1,DWORD PTR [rbp+ymm7*2+0x0],ymm2
vgatherqps xmm1,DWORD PTR [rbp+ymm7*2+0x0],xmm2
vgatherdps xmm11,DWORD PTR [r13+xmm14*2+0x0],xmm12
vgatherqps xmm11,DWORD PTR [r13+xmm14*2+0x0],xmm12
vgatherdps ymm11,DWORD PTR [r13+ymm14*2+0x0],ymm12
vgatherqps xmm11,DWORD PTR [r13+ymm14*2+0x0],xmm12
vgatherdps xmm6,DWORD PTR [xmm4*1+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1+0x298],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x298],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*1+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*1-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*1+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*1+0x298],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*8+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*8-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*8+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm14*8+0x298],xmm5
vpgatherdd xmm1,DWORD PTR [rbp+xmm7*2+0x0],xmm2
vpgatherqd xmm1,DWORD PTR [rbp+xmm7*2+0x0],xmm2
vpgatherdd ymm1,DWORD PTR [rbp+ymm7*2+0x0],ymm2
vpgatherqd xmm1,DWORD PTR [rbp+ymm7*2+0x0],xmm2
vpgatherdd xmm11,DWORD PTR [r13+xmm14*2+0x0],xmm12
vpgatherqd xmm11,DWORD PTR [r13+xmm14*2+0x0],xmm12
vpgatherdd ymm11,DWORD PTR [r13+ymm14*2+0x0],ymm12
vpgatherqd xmm11,DWORD PTR [r13+ymm14*2+0x0],xmm12
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x298],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x298],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*1+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*1-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*1+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*1+0x298],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*8+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*8-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*8+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm14*8+0x298],xmm5
vpgatherdq xmm1,QWORD PTR [rbp+xmm7*2+0x0],xmm2
vpgatherqq xmm1,QWORD PTR [rbp+xmm7*2+0x0],xmm2
vpgatherdq ymm1,QWORD PTR [rbp+xmm7*2+0x0],ymm2
vpgatherqq ymm1,QWORD PTR [rbp+ymm7*2+0x0],ymm2
vpgatherdq xmm11,QWORD PTR [r13+xmm14*2+0x0],xmm12
vpgatherqq xmm11,QWORD PTR [r13+xmm14*2+0x0],xmm12
vpgatherdq ymm11,QWORD PTR [r13+xmm14*2+0x0],ymm12
vpgatherqq ymm11,QWORD PTR [r13+ymm14*2+0x0],ymm12
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x298],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x298],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*1+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*1-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*1+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*1+0x298],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*8+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*8-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*8+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm14*8+0x298],ymm5
