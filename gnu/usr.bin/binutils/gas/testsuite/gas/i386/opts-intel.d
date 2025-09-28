#objdump: -drwMintel,suffix
#name: encoding option (Intel mode)
#source: opts.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	00 d1                	add    cl,dl
[ 	]*[a-f0-9]+:	02 ca                	add.s  cl,dl
[ 	]*[a-f0-9]+:	66 01 d1             	add    cx,dx
[ 	]*[a-f0-9]+:	66 03 ca             	add.s  cx,dx
[ 	]*[a-f0-9]+:	01 d1                	add    ecx,edx
[ 	]*[a-f0-9]+:	03 ca                	add.s  ecx,edx
[ 	]*[a-f0-9]+:	00 d1                	add    cl,dl
[ 	]*[a-f0-9]+:	02 ca                	add.s  cl,dl
[ 	]*[a-f0-9]+:	66 01 d1             	add    cx,dx
[ 	]*[a-f0-9]+:	66 03 ca             	add.s  cx,dx
[ 	]*[a-f0-9]+:	01 d1                	add    ecx,edx
[ 	]*[a-f0-9]+:	03 ca                	add.s  ecx,edx
[ 	]*[a-f0-9]+:	10 d1                	adc    cl,dl
[ 	]*[a-f0-9]+:	12 ca                	adc.s  cl,dl
[ 	]*[a-f0-9]+:	66 11 d1             	adc    cx,dx
[ 	]*[a-f0-9]+:	66 13 ca             	adc.s  cx,dx
[ 	]*[a-f0-9]+:	11 d1                	adc    ecx,edx
[ 	]*[a-f0-9]+:	13 ca                	adc.s  ecx,edx
[ 	]*[a-f0-9]+:	10 d1                	adc    cl,dl
[ 	]*[a-f0-9]+:	12 ca                	adc.s  cl,dl
[ 	]*[a-f0-9]+:	66 11 d1             	adc    cx,dx
[ 	]*[a-f0-9]+:	66 13 ca             	adc.s  cx,dx
[ 	]*[a-f0-9]+:	11 d1                	adc    ecx,edx
[ 	]*[a-f0-9]+:	13 ca                	adc.s  ecx,edx
[ 	]*[a-f0-9]+:	20 d1                	and    cl,dl
[ 	]*[a-f0-9]+:	22 ca                	and.s  cl,dl
[ 	]*[a-f0-9]+:	66 21 d1             	and    cx,dx
[ 	]*[a-f0-9]+:	66 23 ca             	and.s  cx,dx
[ 	]*[a-f0-9]+:	21 d1                	and    ecx,edx
[ 	]*[a-f0-9]+:	23 ca                	and.s  ecx,edx
[ 	]*[a-f0-9]+:	20 d1                	and    cl,dl
[ 	]*[a-f0-9]+:	22 ca                	and.s  cl,dl
[ 	]*[a-f0-9]+:	66 21 d1             	and    cx,dx
[ 	]*[a-f0-9]+:	66 23 ca             	and.s  cx,dx
[ 	]*[a-f0-9]+:	21 d1                	and    ecx,edx
[ 	]*[a-f0-9]+:	23 ca                	and.s  ecx,edx
[ 	]*[a-f0-9]+:	38 d1                	cmp    cl,dl
[ 	]*[a-f0-9]+:	3a ca                	cmp.s  cl,dl
[ 	]*[a-f0-9]+:	66 39 d1             	cmp    cx,dx
[ 	]*[a-f0-9]+:	66 3b ca             	cmp.s  cx,dx
[ 	]*[a-f0-9]+:	39 d1                	cmp    ecx,edx
[ 	]*[a-f0-9]+:	3b ca                	cmp.s  ecx,edx
[ 	]*[a-f0-9]+:	38 d1                	cmp    cl,dl
[ 	]*[a-f0-9]+:	3a ca                	cmp.s  cl,dl
[ 	]*[a-f0-9]+:	66 39 d1             	cmp    cx,dx
[ 	]*[a-f0-9]+:	66 3b ca             	cmp.s  cx,dx
[ 	]*[a-f0-9]+:	39 d1                	cmp    ecx,edx
[ 	]*[a-f0-9]+:	3b ca                	cmp.s  ecx,edx
[ 	]*[a-f0-9]+:	88 d1                	mov    cl,dl
[ 	]*[a-f0-9]+:	8a ca                	mov.s  cl,dl
[ 	]*[a-f0-9]+:	66 89 d1             	mov    cx,dx
[ 	]*[a-f0-9]+:	66 8b ca             	mov.s  cx,dx
[ 	]*[a-f0-9]+:	89 d1                	mov    ecx,edx
[ 	]*[a-f0-9]+:	8b ca                	mov.s  ecx,edx
[ 	]*[a-f0-9]+:	88 d1                	mov    cl,dl
[ 	]*[a-f0-9]+:	8a ca                	mov.s  cl,dl
[ 	]*[a-f0-9]+:	66 89 d1             	mov    cx,dx
[ 	]*[a-f0-9]+:	66 8b ca             	mov.s  cx,dx
[ 	]*[a-f0-9]+:	89 d1                	mov    ecx,edx
[ 	]*[a-f0-9]+:	8b ca                	mov.s  ecx,edx
[ 	]*[a-f0-9]+:	08 d1                	or     cl,dl
[ 	]*[a-f0-9]+:	0a ca                	or.s   cl,dl
[ 	]*[a-f0-9]+:	66 09 d1             	or     cx,dx
[ 	]*[a-f0-9]+:	66 0b ca             	or.s   cx,dx
[ 	]*[a-f0-9]+:	09 d1                	or     ecx,edx
[ 	]*[a-f0-9]+:	0b ca                	or.s   ecx,edx
[ 	]*[a-f0-9]+:	08 d1                	or     cl,dl
[ 	]*[a-f0-9]+:	0a ca                	or.s   cl,dl
[ 	]*[a-f0-9]+:	66 09 d1             	or     cx,dx
[ 	]*[a-f0-9]+:	66 0b ca             	or.s   cx,dx
[ 	]*[a-f0-9]+:	09 d1                	or     ecx,edx
[ 	]*[a-f0-9]+:	0b ca                	or.s   ecx,edx
[ 	]*[a-f0-9]+:	18 d1                	sbb    cl,dl
[ 	]*[a-f0-9]+:	1a ca                	sbb.s  cl,dl
[ 	]*[a-f0-9]+:	66 19 d1             	sbb    cx,dx
[ 	]*[a-f0-9]+:	66 1b ca             	sbb.s  cx,dx
[ 	]*[a-f0-9]+:	19 d1                	sbb    ecx,edx
[ 	]*[a-f0-9]+:	1b ca                	sbb.s  ecx,edx
[ 	]*[a-f0-9]+:	18 d1                	sbb    cl,dl
[ 	]*[a-f0-9]+:	1a ca                	sbb.s  cl,dl
[ 	]*[a-f0-9]+:	66 19 d1             	sbb    cx,dx
[ 	]*[a-f0-9]+:	66 1b ca             	sbb.s  cx,dx
[ 	]*[a-f0-9]+:	19 d1                	sbb    ecx,edx
[ 	]*[a-f0-9]+:	1b ca                	sbb.s  ecx,edx
[ 	]*[a-f0-9]+:	28 d1                	sub    cl,dl
[ 	]*[a-f0-9]+:	2a ca                	sub.s  cl,dl
[ 	]*[a-f0-9]+:	66 29 d1             	sub    cx,dx
[ 	]*[a-f0-9]+:	66 2b ca             	sub.s  cx,dx
[ 	]*[a-f0-9]+:	29 d1                	sub    ecx,edx
[ 	]*[a-f0-9]+:	2b ca                	sub.s  ecx,edx
[ 	]*[a-f0-9]+:	28 d1                	sub    cl,dl
[ 	]*[a-f0-9]+:	2a ca                	sub.s  cl,dl
[ 	]*[a-f0-9]+:	66 29 d1             	sub    cx,dx
[ 	]*[a-f0-9]+:	66 2b ca             	sub.s  cx,dx
[ 	]*[a-f0-9]+:	29 d1                	sub    ecx,edx
[ 	]*[a-f0-9]+:	2b ca                	sub.s  ecx,edx
[ 	]*[a-f0-9]+:	30 d1                	xor    cl,dl
[ 	]*[a-f0-9]+:	32 ca                	xor.s  cl,dl
[ 	]*[a-f0-9]+:	66 31 d1             	xor    cx,dx
[ 	]*[a-f0-9]+:	66 33 ca             	xor.s  cx,dx
[ 	]*[a-f0-9]+:	31 d1                	xor    ecx,edx
[ 	]*[a-f0-9]+:	33 ca                	xor.s  ecx,edx
[ 	]*[a-f0-9]+:	30 d1                	xor    cl,dl
[ 	]*[a-f0-9]+:	32 ca                	xor.s  cl,dl
[ 	]*[a-f0-9]+:	66 31 d1             	xor    cx,dx
[ 	]*[a-f0-9]+:	66 33 ca             	xor.s  cx,dx
[ 	]*[a-f0-9]+:	31 d1                	xor    ecx,edx
[ 	]*[a-f0-9]+:	33 ca                	xor.s  ecx,edx
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 e6          	vmovapd.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 e6          	vmovaps.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f e6          	vmovdqa.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f e6          	vmovdqu.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 e6          	vmovupd.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 e6          	vmovups.s ymm6,ymm4
[ 	]*[a-f0-9]+:	66 0f 28 f4          	movapd xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 29 e6          	movapd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 28 f4             	movaps xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 29 e6             	movaps.s xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 6f f4          	movdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 7f e6          	movdqa.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 6f f4          	movdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 7f e6          	movdqu.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 7e f4          	movq   xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f d6 e6          	movq.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f2 0f 10 f4          	movsd  xmm6,xmm4
[ 	]*[a-f0-9]+:	f2 0f 11 e6          	movsd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 10 f4          	movss  xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 11 e6          	movss.s xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 10 f4          	movupd xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 11 e6          	movupd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 10 f4             	movups xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 11 e6             	movups.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 11 e2          	vmovsd.s xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 11 e2          	vmovss.s xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 6f e0             	movq   mm4,mm0
[ 	]*[a-f0-9]+:	0f 7f c4             	movq.s mm4,mm0
[ 	]*[a-f0-9]+:	66 0f 1a d1          	bndmov bnd2,bnd1
[ 	]*[a-f0-9]+:	66 0f 1b ca          	bndmov.s bnd2,bnd1
[ 	]*[a-f0-9]+:	00 d1                	add    cl,dl
[ 	]*[a-f0-9]+:	02 ca                	add.s  cl,dl
[ 	]*[a-f0-9]+:	66 01 d1             	add    cx,dx
[ 	]*[a-f0-9]+:	66 03 ca             	add.s  cx,dx
[ 	]*[a-f0-9]+:	01 d1                	add    ecx,edx
[ 	]*[a-f0-9]+:	03 ca                	add.s  ecx,edx
[ 	]*[a-f0-9]+:	10 d1                	adc    cl,dl
[ 	]*[a-f0-9]+:	12 ca                	adc.s  cl,dl
[ 	]*[a-f0-9]+:	66 11 d1             	adc    cx,dx
[ 	]*[a-f0-9]+:	66 13 ca             	adc.s  cx,dx
[ 	]*[a-f0-9]+:	11 d1                	adc    ecx,edx
[ 	]*[a-f0-9]+:	13 ca                	adc.s  ecx,edx
[ 	]*[a-f0-9]+:	20 d1                	and    cl,dl
[ 	]*[a-f0-9]+:	22 ca                	and.s  cl,dl
[ 	]*[a-f0-9]+:	66 21 d1             	and    cx,dx
[ 	]*[a-f0-9]+:	66 23 ca             	and.s  cx,dx
[ 	]*[a-f0-9]+:	21 d1                	and    ecx,edx
[ 	]*[a-f0-9]+:	23 ca                	and.s  ecx,edx
[ 	]*[a-f0-9]+:	38 d1                	cmp    cl,dl
[ 	]*[a-f0-9]+:	3a ca                	cmp.s  cl,dl
[ 	]*[a-f0-9]+:	66 39 d1             	cmp    cx,dx
[ 	]*[a-f0-9]+:	66 3b ca             	cmp.s  cx,dx
[ 	]*[a-f0-9]+:	39 d1                	cmp    ecx,edx
[ 	]*[a-f0-9]+:	3b ca                	cmp.s  ecx,edx
[ 	]*[a-f0-9]+:	88 d1                	mov    cl,dl
[ 	]*[a-f0-9]+:	8a ca                	mov.s  cl,dl
[ 	]*[a-f0-9]+:	66 89 d1             	mov    cx,dx
[ 	]*[a-f0-9]+:	66 8b ca             	mov.s  cx,dx
[ 	]*[a-f0-9]+:	89 d1                	mov    ecx,edx
[ 	]*[a-f0-9]+:	8b ca                	mov.s  ecx,edx
[ 	]*[a-f0-9]+:	08 d1                	or     cl,dl
[ 	]*[a-f0-9]+:	0a ca                	or.s   cl,dl
[ 	]*[a-f0-9]+:	66 09 d1             	or     cx,dx
[ 	]*[a-f0-9]+:	66 0b ca             	or.s   cx,dx
[ 	]*[a-f0-9]+:	09 d1                	or     ecx,edx
[ 	]*[a-f0-9]+:	0b ca                	or.s   ecx,edx
[ 	]*[a-f0-9]+:	18 d1                	sbb    cl,dl
[ 	]*[a-f0-9]+:	1a ca                	sbb.s  cl,dl
[ 	]*[a-f0-9]+:	66 19 d1             	sbb    cx,dx
[ 	]*[a-f0-9]+:	66 1b ca             	sbb.s  cx,dx
[ 	]*[a-f0-9]+:	19 d1                	sbb    ecx,edx
[ 	]*[a-f0-9]+:	1b ca                	sbb.s  ecx,edx
[ 	]*[a-f0-9]+:	28 d1                	sub    cl,dl
[ 	]*[a-f0-9]+:	2a ca                	sub.s  cl,dl
[ 	]*[a-f0-9]+:	66 29 d1             	sub    cx,dx
[ 	]*[a-f0-9]+:	66 2b ca             	sub.s  cx,dx
[ 	]*[a-f0-9]+:	29 d1                	sub    ecx,edx
[ 	]*[a-f0-9]+:	2b ca                	sub.s  ecx,edx
[ 	]*[a-f0-9]+:	30 d1                	xor    cl,dl
[ 	]*[a-f0-9]+:	32 ca                	xor.s  cl,dl
[ 	]*[a-f0-9]+:	66 31 d1             	xor    cx,dx
[ 	]*[a-f0-9]+:	66 33 ca             	xor.s  cx,dx
[ 	]*[a-f0-9]+:	31 d1                	xor    ecx,edx
[ 	]*[a-f0-9]+:	33 ca                	xor.s  ecx,edx
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 e6          	vmovapd.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 e6          	vmovaps.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f e6          	vmovdqa.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f e6          	vmovdqu.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 e6          	vmovupd.s ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 e6          	vmovups.s ymm6,ymm4
[ 	]*[a-f0-9]+:	66 0f 28 f4          	movapd xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 29 e6          	movapd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 28 f4             	movaps xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 29 e6             	movaps.s xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 6f f4          	movdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 7f e6          	movdqa.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 6f f4          	movdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 7f e6          	movdqu.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 7e f4          	movq   xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f d6 e6          	movq.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f2 0f 10 f4          	movsd  xmm6,xmm4
[ 	]*[a-f0-9]+:	f2 0f 11 e6          	movsd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 10 f4          	movss  xmm6,xmm4
[ 	]*[a-f0-9]+:	f3 0f 11 e6          	movss.s xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 10 f4          	movupd xmm6,xmm4
[ 	]*[a-f0-9]+:	66 0f 11 e6          	movupd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 10 f4             	movups xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 11 e6             	movups.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 11 e2          	vmovsd.s xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 11 e2          	vmovss.s xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	0f 6f e0             	movq   mm4,mm0
[ 	]*[a-f0-9]+:	0f 7f c4             	movq.s mm4,mm0
[ 	]*[a-f0-9]+:	66 0f 1a ca          	bndmov bnd1,bnd2
[ 	]*[a-f0-9]+:	66 0f 1b d1          	bndmov.s bnd1,bnd2
#pass
