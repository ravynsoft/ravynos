#as:
#objdump: -dw
#name: i386 AVX GATHER insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd %ymm2,0x0\(%ebp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%ebp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%ebp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq %ymm2,0x0\(%ebp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd %ymm2,0x0\(%ebp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%ebp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%ebp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq %xmm2,0x0\(%ebp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq %ymm2,0x0\(%ebp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq %ymm2,0x0\(%ebp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,8\),%ymm6
#pass
