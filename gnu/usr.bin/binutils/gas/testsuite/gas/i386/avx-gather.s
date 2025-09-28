# Check 32bit AVX gather instructions

	.text
_start:
	vgatherdpd %xmm2, (%ebp, %xmm7, 2),%xmm1
	vgatherqpd %xmm2, (%ebp, %xmm7, 2),%xmm1
	vgatherdpd %ymm2, (%ebp, %xmm7, 2),%ymm1
	vgatherqpd %ymm2, (%ebp, %ymm7, 2),%ymm1

	vgatherdpd %ymm5,0x8(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm4,1),%ymm6
	vgatherdpd %ymm5,0x8(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,-0x8(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,(,%xmm4,8),%ymm6
	vgatherdpd %ymm5,0x298(,%xmm4,8),%ymm6

	vgatherdps %xmm2, (%ebp, %xmm7, 2),%xmm1
	vgatherqps %xmm2, (%ebp, %xmm7, 2),%xmm1
	vgatherdps %ymm2, (%ebp, %ymm7, 2),%ymm1
	vgatherqps %xmm2, (%ebp, %ymm7, 2),%xmm1

	vgatherdps %xmm5,0x8(,%xmm4,1),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm4,1),%xmm6
	vgatherdps %xmm5,(,%xmm4,1),%xmm6
	vgatherdps %xmm5,0x298(,%xmm4,1),%xmm6
	vgatherdps %xmm5,0x8(,%xmm4,8),%xmm6
	vgatherdps %xmm5,-0x8(,%xmm4,8),%xmm6
	vgatherdps %xmm5,(,%xmm4,8),%xmm6
	vgatherdps %xmm5,0x298(,%xmm4,8),%xmm6

	vpgatherdd %xmm2, (%ebp, %xmm7, 2),%xmm1
	vpgatherqd %xmm2, (%ebp, %xmm7, 2),%xmm1
	vpgatherdd %ymm2, (%ebp, %ymm7, 2),%ymm1
	vpgatherqd %xmm2, (%ebp, %ymm7, 2),%xmm1

	vpgatherdd %xmm5,0x8(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm4,1),%xmm6
	vpgatherdd %xmm5,0x8(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,-0x8(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,(,%xmm4,8),%xmm6
	vpgatherdd %xmm5,0x298(,%xmm4,8),%xmm6

	vpgatherdq %xmm2, (%ebp, %xmm7, 2),%xmm1
	vpgatherqq %xmm2, (%ebp, %xmm7, 2),%xmm1
	vpgatherdq %ymm2, (%ebp, %xmm7, 2),%ymm1
	vpgatherqq %ymm2, (%ebp, %ymm7, 2),%ymm1

	vpgatherdq %ymm5,0x8(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm4,1),%ymm6
	vpgatherdq %ymm5,0x8(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,-0x8(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,(,%xmm4,8),%ymm6
	vpgatherdq %ymm5,0x298(,%xmm4,8),%ymm6

	.intel_syntax noprefix
vgatherdpd xmm1,QWORD PTR [ebp+xmm7*2+0x0],xmm2
vgatherqpd xmm1,QWORD PTR [ebp+xmm7*2+0x0],xmm2
vgatherdpd ymm1,QWORD PTR [ebp+xmm7*2+0x0],ymm2
vgatherqpd ymm1,QWORD PTR [ebp+ymm7*2+0x0],ymm2
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*1+0x298],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8-0x8],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x0],ymm5
vgatherdpd ymm6,QWORD PTR [xmm4*8+0x298],ymm5
vgatherdps xmm1,DWORD PTR [ebp+xmm7*2+0x0],xmm2
vgatherqps xmm1,DWORD PTR [ebp+xmm7*2+0x0],xmm2
vgatherdps ymm1,DWORD PTR [ebp+ymm7*2+0x0],ymm2
vgatherqps xmm1,DWORD PTR [ebp+ymm7*2+0x0],xmm2
vgatherdps xmm6,DWORD PTR [xmm4*1+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*1+0x298],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8-0x8],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x0],xmm5
vgatherdps xmm6,DWORD PTR [xmm4*8+0x298],xmm5
vpgatherdd xmm1,DWORD PTR [ebp+xmm7*2+0x0],xmm2
vpgatherqd xmm1,DWORD PTR [ebp+xmm7*2+0x0],xmm2
vpgatherdd ymm1,DWORD PTR [ebp+ymm7*2+0x0],ymm2
vpgatherqd xmm1,DWORD PTR [ebp+ymm7*2+0x0],xmm2
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*1+0x298],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8-0x8],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x0],xmm5
vpgatherdd xmm6,DWORD PTR [xmm4*8+0x298],xmm5
vpgatherdq xmm1,QWORD PTR [ebp+xmm7*2+0x0],xmm2
vpgatherqq xmm1,QWORD PTR [ebp+xmm7*2+0x0],xmm2
vpgatherdq ymm1,QWORD PTR [ebp+xmm7*2+0x0],ymm2
vpgatherqq ymm1,QWORD PTR [ebp+ymm7*2+0x0],ymm2
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*1+0x298],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8-0x8],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x0],ymm5
vpgatherdq ymm6,QWORD PTR [xmm4*8+0x298],ymm5
