#as:
#objdump: -dw
#name: x86-64 AVX GATHER insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd %ymm2,0x0\(%rbp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 02 99 92 5c 75 00 	vgatherdpd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 99 93 5c 75 00 	vgatherqpd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 9d 92 5c 75 00 	vgatherdpd %ymm12,0x0\(%r13,%xmm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 9d 93 5c 75 00 	vgatherqpd %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%rbp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 02 19 92 5c 75 00 	vgatherdps %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 19 93 5c 75 00 	vgatherqps %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 1d 92 5c 75 00 	vgatherdps %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 1d 93 5c 75 00 	vgatherqps %xmm12,0x0\(%r13,%ymm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%rbp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 02 19 90 5c 75 00 	vpgatherdd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 19 91 5c 75 00 	vpgatherqd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 1d 90 5c 75 00 	vpgatherdd %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 1d 91 5c 75 00 	vpgatherqd %xmm12,0x0\(%r13,%ymm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq %ymm2,0x0\(%rbp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 02 99 90 5c 75 00 	vpgatherdq %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 99 91 5c 75 00 	vpgatherqq %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 9d 90 5c 75 00 	vpgatherdq %ymm12,0x0\(%r13,%xmm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 9d 91 5c 75 00 	vpgatherqq %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 e9 92 4c 7d 00 	vgatherdpd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 93 4c 7d 00 	vgatherqpd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 92 4c 7d 00 	vgatherdpd %ymm2,0x0\(%rbp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 93 4c 7d 00 	vgatherqpd %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 02 99 92 5c 75 00 	vgatherdpd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 99 93 5c 75 00 	vgatherqpd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 9d 92 5c 75 00 	vgatherdpd %ymm12,0x0\(%r13,%xmm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 9d 93 5c 75 00 	vgatherqpd %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 25 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 92 34 e5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 35 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 08 00 00 00 	vgatherdpd %ymm5,0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 f8 ff ff ff 	vgatherdpd %ymm5,-0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 00 00 00 00 	vgatherdpd %ymm5,0x0\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 92 34 f5 98 02 00 00 	vgatherdpd %ymm5,0x298\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 69 92 4c 7d 00 	vgatherdps %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 92 4c 7d 00 	vgatherdps %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 93 4c 7d 00 	vgatherqps %xmm2,0x0\(%rbp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 02 19 92 5c 75 00 	vgatherdps %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 19 93 5c 75 00 	vgatherqps %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 1d 92 5c 75 00 	vgatherdps %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 1d 93 5c 75 00 	vgatherqps %xmm12,0x0\(%r13,%ymm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 25 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 92 34 e5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 35 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 08 00 00 00 	vgatherdps %xmm5,0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 f8 ff ff ff 	vgatherdps %xmm5,-0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 00 00 00 00 	vgatherdps %xmm5,0x0\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 92 34 f5 98 02 00 00 	vgatherdps %xmm5,0x298\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 69 90 4c 7d 00 	vpgatherdd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 69 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 6d 90 4c 7d 00 	vpgatherdd %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 6d 91 4c 7d 00 	vpgatherqd %xmm2,0x0\(%rbp,%ymm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 02 19 90 5c 75 00 	vpgatherdd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 19 91 5c 75 00 	vpgatherqd %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 1d 90 5c 75 00 	vpgatherdd %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 1d 91 5c 75 00 	vpgatherqd %xmm12,0x0\(%r13,%ymm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 25 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 90 34 e5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm4,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 35 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm14,1\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 08 00 00 00 	vpgatherdd %xmm5,0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 f8 ff ff ff 	vpgatherdd %xmm5,-0x8\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 00 00 00 00 	vpgatherdd %xmm5,0x0\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 90 34 f5 98 02 00 00 	vpgatherdd %xmm5,0x298\(,%xmm14,8\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 e9 90 4c 7d 00 	vpgatherdq %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 e9 91 4c 7d 00 	vpgatherqq %xmm2,0x0\(%rbp,%xmm7,2\),%xmm1
[ 	]*[a-f0-9]+:	c4 e2 ed 90 4c 7d 00 	vpgatherdq %ymm2,0x0\(%rbp,%xmm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 e2 ed 91 4c 7d 00 	vpgatherqq %ymm2,0x0\(%rbp,%ymm7,2\),%ymm1
[ 	]*[a-f0-9]+:	c4 02 99 90 5c 75 00 	vpgatherdq %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 99 91 5c 75 00 	vpgatherqq %xmm12,0x0\(%r13,%xmm14,2\),%xmm11
[ 	]*[a-f0-9]+:	c4 02 9d 90 5c 75 00 	vpgatherdq %ymm12,0x0\(%r13,%xmm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 02 9d 91 5c 75 00 	vpgatherqq %ymm12,0x0\(%r13,%ymm14,2\),%ymm11
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 25 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 d5 90 34 e5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm4,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 35 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm14,1\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 08 00 00 00 	vpgatherdq %ymm5,0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 f8 ff ff ff 	vpgatherdq %ymm5,-0x8\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 00 00 00 00 	vpgatherdq %ymm5,0x0\(,%xmm14,8\),%ymm6
[ 	]*[a-f0-9]+:	c4 a2 d5 90 34 f5 98 02 00 00 	vpgatherdq %ymm5,0x298\(,%xmm14,8\),%ymm6
#pass
