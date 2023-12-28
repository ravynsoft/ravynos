#as:
#objdump: -dwMintel
#name: i386 AVX GATHER insns (Intel disassembly)
#source: avx-gather.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd ymm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd ymm1,QWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps ymm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd ymm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq ymm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq ymm1,QWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd ymm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd ymm1,QWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps ymm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps xmm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd ymm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd xmm1,DWORD PTR \[ebp\+ymm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*1\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8-0x8\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x0\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd xmm6,DWORD PTR \[xmm4\*8\+0x298\],xmm5
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq xmm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],xmm2
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq ymm1,QWORD PTR \[ebp\+xmm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq ymm1,QWORD PTR \[ebp\+ymm7\*2\+0x0\],ymm2
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*1\+0x298\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8-0x8\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x0\],ymm5
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq ymm6,QWORD PTR \[xmm4\*8\+0x298\],ymm5
#pass
