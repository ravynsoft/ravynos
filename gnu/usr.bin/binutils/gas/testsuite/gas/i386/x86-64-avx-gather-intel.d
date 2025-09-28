#as:
#objdump: -dwMintel
#name: x86-64 AVX GATHER insns (Intel disassembly)
#source: x86-64-avx-gather.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd ymm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd ymm1,QWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 02 99 92 5c 75 00 	vgatherdpd xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 99 93 5c 75 00 	vgatherqpd xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 9d 92 5c 75 00 	vgatherdpd ymm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 9d 93 5c 75 00 	vgatherqpd ymm11,QWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps ymm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 02 19 92 5c 75 00 	vgatherdps xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 19 93 5c 75 00 	vgatherqps xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 1d 92 5c 75 00 	vgatherdps ymm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 1d 93 5c 75 00 	vgatherqps xmm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm14\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm14\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd ymm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 02 19 90 5c 75 00 	vpgatherdd xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 19 91 5c 75 00 	vpgatherqd xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 1d 90 5c 75 00 	vpgatherdd ymm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 1d 91 5c 75 00 	vpgatherqd xmm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq ymm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq ymm1,QWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 02 99 90 5c 75 00 	vpgatherdq xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 99 91 5c 75 00 	vpgatherqq xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 9d 90 5c 75 00 	vpgatherdq ymm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 9d 91 5c 75 00 	vpgatherqq ymm11,QWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd ymm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd ymm1,QWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 02 99 92 5c 75 00 	vgatherdpd xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 99 93 5c 75 00 	vgatherqpd xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 9d 92 5c 75 00 	vgatherdpd ymm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 9d 93 5c 75 00 	vgatherqpd ymm11,QWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm14\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps ymm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 02 19 92 5c 75 00 	vgatherdps xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 19 93 5c 75 00 	vgatherqps xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 1d 92 5c 75 00 	vgatherdps ymm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 1d 93 5c 75 00 	vgatherqps xmm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm14\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm14\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm14\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd ymm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[rbp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 02 19 90 5c 75 00 	vpgatherdd xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 19 91 5c 75 00 	vpgatherqd xmm11,DWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 1d 90 5c 75 00 	vpgatherdd ymm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 1d 91 5c 75 00 	vpgatherqd xmm11,DWORD PTR \[r13\+ymm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm14\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq xmm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq ymm1,QWORD PTR \[rbp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq ymm1,QWORD PTR \[rbp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 02 99 90 5c 75 00 	vpgatherdq xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 99 91 5c 75 00 	vpgatherqq xmm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],xmm12
[ 	]*[a-f0-9]+:	c4 02 9d 90 5c 75 00 	vpgatherdq ymm11,QWORD PTR \[r13\+xmm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 02 9d 91 5c 75 00 	vpgatherqq ymm11,QWORD PTR \[r13\+ymm14\*2\+0x0\],ymm12
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm14\*8\+0x298\],ymm5
#pass
