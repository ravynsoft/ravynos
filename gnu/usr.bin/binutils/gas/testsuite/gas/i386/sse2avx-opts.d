#as: -msse2avx
#objdump: -drwMsuffix
#name: encoding option with -msse2avx
#source: opts.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	00 d1                	addb   %dl,%cl
[ 	]*[a-f0-9]+:	02 ca                	addb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 01 d1             	addw   %dx,%cx
[ 	]*[a-f0-9]+:	66 03 ca             	addw.s %dx,%cx
[ 	]*[a-f0-9]+:	01 d1                	addl   %edx,%ecx
[ 	]*[a-f0-9]+:	03 ca                	addl.s %edx,%ecx
[ 	]*[a-f0-9]+:	00 d1                	addb   %dl,%cl
[ 	]*[a-f0-9]+:	02 ca                	addb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 01 d1             	addw   %dx,%cx
[ 	]*[a-f0-9]+:	66 03 ca             	addw.s %dx,%cx
[ 	]*[a-f0-9]+:	01 d1                	addl   %edx,%ecx
[ 	]*[a-f0-9]+:	03 ca                	addl.s %edx,%ecx
[ 	]*[a-f0-9]+:	10 d1                	adcb   %dl,%cl
[ 	]*[a-f0-9]+:	12 ca                	adcb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 11 d1             	adcw   %dx,%cx
[ 	]*[a-f0-9]+:	66 13 ca             	adcw.s %dx,%cx
[ 	]*[a-f0-9]+:	11 d1                	adcl   %edx,%ecx
[ 	]*[a-f0-9]+:	13 ca                	adcl.s %edx,%ecx
[ 	]*[a-f0-9]+:	10 d1                	adcb   %dl,%cl
[ 	]*[a-f0-9]+:	12 ca                	adcb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 11 d1             	adcw   %dx,%cx
[ 	]*[a-f0-9]+:	66 13 ca             	adcw.s %dx,%cx
[ 	]*[a-f0-9]+:	11 d1                	adcl   %edx,%ecx
[ 	]*[a-f0-9]+:	13 ca                	adcl.s %edx,%ecx
[ 	]*[a-f0-9]+:	20 d1                	andb   %dl,%cl
[ 	]*[a-f0-9]+:	22 ca                	andb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 21 d1             	andw   %dx,%cx
[ 	]*[a-f0-9]+:	66 23 ca             	andw.s %dx,%cx
[ 	]*[a-f0-9]+:	21 d1                	andl   %edx,%ecx
[ 	]*[a-f0-9]+:	23 ca                	andl.s %edx,%ecx
[ 	]*[a-f0-9]+:	20 d1                	andb   %dl,%cl
[ 	]*[a-f0-9]+:	22 ca                	andb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 21 d1             	andw   %dx,%cx
[ 	]*[a-f0-9]+:	66 23 ca             	andw.s %dx,%cx
[ 	]*[a-f0-9]+:	21 d1                	andl   %edx,%ecx
[ 	]*[a-f0-9]+:	23 ca                	andl.s %edx,%ecx
[ 	]*[a-f0-9]+:	38 d1                	cmpb   %dl,%cl
[ 	]*[a-f0-9]+:	3a ca                	cmpb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 39 d1             	cmpw   %dx,%cx
[ 	]*[a-f0-9]+:	66 3b ca             	cmpw.s %dx,%cx
[ 	]*[a-f0-9]+:	39 d1                	cmpl   %edx,%ecx
[ 	]*[a-f0-9]+:	3b ca                	cmpl.s %edx,%ecx
[ 	]*[a-f0-9]+:	38 d1                	cmpb   %dl,%cl
[ 	]*[a-f0-9]+:	3a ca                	cmpb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 39 d1             	cmpw   %dx,%cx
[ 	]*[a-f0-9]+:	66 3b ca             	cmpw.s %dx,%cx
[ 	]*[a-f0-9]+:	39 d1                	cmpl   %edx,%ecx
[ 	]*[a-f0-9]+:	3b ca                	cmpl.s %edx,%ecx
[ 	]*[a-f0-9]+:	88 d1                	movb   %dl,%cl
[ 	]*[a-f0-9]+:	8a ca                	movb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 89 d1             	movw   %dx,%cx
[ 	]*[a-f0-9]+:	66 8b ca             	movw.s %dx,%cx
[ 	]*[a-f0-9]+:	89 d1                	movl   %edx,%ecx
[ 	]*[a-f0-9]+:	8b ca                	movl.s %edx,%ecx
[ 	]*[a-f0-9]+:	88 d1                	movb   %dl,%cl
[ 	]*[a-f0-9]+:	8a ca                	movb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 89 d1             	movw   %dx,%cx
[ 	]*[a-f0-9]+:	66 8b ca             	movw.s %dx,%cx
[ 	]*[a-f0-9]+:	89 d1                	movl   %edx,%ecx
[ 	]*[a-f0-9]+:	8b ca                	movl.s %edx,%ecx
[ 	]*[a-f0-9]+:	08 d1                	orb    %dl,%cl
[ 	]*[a-f0-9]+:	0a ca                	orb.s  %dl,%cl
[ 	]*[a-f0-9]+:	66 09 d1             	orw    %dx,%cx
[ 	]*[a-f0-9]+:	66 0b ca             	orw.s  %dx,%cx
[ 	]*[a-f0-9]+:	09 d1                	orl    %edx,%ecx
[ 	]*[a-f0-9]+:	0b ca                	orl.s  %edx,%ecx
[ 	]*[a-f0-9]+:	08 d1                	orb    %dl,%cl
[ 	]*[a-f0-9]+:	0a ca                	orb.s  %dl,%cl
[ 	]*[a-f0-9]+:	66 09 d1             	orw    %dx,%cx
[ 	]*[a-f0-9]+:	66 0b ca             	orw.s  %dx,%cx
[ 	]*[a-f0-9]+:	09 d1                	orl    %edx,%ecx
[ 	]*[a-f0-9]+:	0b ca                	orl.s  %edx,%ecx
[ 	]*[a-f0-9]+:	18 d1                	sbbb   %dl,%cl
[ 	]*[a-f0-9]+:	1a ca                	sbbb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 19 d1             	sbbw   %dx,%cx
[ 	]*[a-f0-9]+:	66 1b ca             	sbbw.s %dx,%cx
[ 	]*[a-f0-9]+:	19 d1                	sbbl   %edx,%ecx
[ 	]*[a-f0-9]+:	1b ca                	sbbl.s %edx,%ecx
[ 	]*[a-f0-9]+:	18 d1                	sbbb   %dl,%cl
[ 	]*[a-f0-9]+:	1a ca                	sbbb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 19 d1             	sbbw   %dx,%cx
[ 	]*[a-f0-9]+:	66 1b ca             	sbbw.s %dx,%cx
[ 	]*[a-f0-9]+:	19 d1                	sbbl   %edx,%ecx
[ 	]*[a-f0-9]+:	1b ca                	sbbl.s %edx,%ecx
[ 	]*[a-f0-9]+:	28 d1                	subb   %dl,%cl
[ 	]*[a-f0-9]+:	2a ca                	subb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 29 d1             	subw   %dx,%cx
[ 	]*[a-f0-9]+:	66 2b ca             	subw.s %dx,%cx
[ 	]*[a-f0-9]+:	29 d1                	subl   %edx,%ecx
[ 	]*[a-f0-9]+:	2b ca                	subl.s %edx,%ecx
[ 	]*[a-f0-9]+:	28 d1                	subb   %dl,%cl
[ 	]*[a-f0-9]+:	2a ca                	subb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 29 d1             	subw   %dx,%cx
[ 	]*[a-f0-9]+:	66 2b ca             	subw.s %dx,%cx
[ 	]*[a-f0-9]+:	29 d1                	subl   %edx,%ecx
[ 	]*[a-f0-9]+:	2b ca                	subl.s %edx,%ecx
[ 	]*[a-f0-9]+:	30 d1                	xorb   %dl,%cl
[ 	]*[a-f0-9]+:	32 ca                	xorb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 31 d1             	xorw   %dx,%cx
[ 	]*[a-f0-9]+:	66 33 ca             	xorw.s %dx,%cx
[ 	]*[a-f0-9]+:	31 d1                	xorl   %edx,%ecx
[ 	]*[a-f0-9]+:	33 ca                	xorl.s %edx,%ecx
[ 	]*[a-f0-9]+:	30 d1                	xorb   %dl,%cl
[ 	]*[a-f0-9]+:	32 ca                	xorb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 31 d1             	xorw   %dx,%cx
[ 	]*[a-f0-9]+:	66 33 ca             	xorw.s %dx,%cx
[ 	]*[a-f0-9]+:	31 d1                	xorl   %edx,%ecx
[ 	]*[a-f0-9]+:	33 ca                	xorl.s %edx,%ecx
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 29 e6          	vmovapd.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 29 e6          	vmovaps.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 7f e6          	vmovdqa.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 7f e6          	vmovdqu.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 11 e6          	vmovupd.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 11 e6          	vmovups.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 f4          	vmovsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 11 e6          	vmovsd.s %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 10 f4          	vmovss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 11 e6          	vmovss.s %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 11 e2          	vmovsd.s %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 11 e2          	vmovss.s %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	0f 6f e0             	movq   %mm0,%mm4
[ 	]*[a-f0-9]+:	0f 7f c4             	movq.s %mm0,%mm4
[ 	]*[a-f0-9]+:	66 0f 1a d1          	bndmov %bnd1,%bnd2
[ 	]*[a-f0-9]+:	66 0f 1b ca          	bndmov.s %bnd1,%bnd2
[ 	]*[a-f0-9]+:	00 d1                	addb   %dl,%cl
[ 	]*[a-f0-9]+:	02 ca                	addb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 01 d1             	addw   %dx,%cx
[ 	]*[a-f0-9]+:	66 03 ca             	addw.s %dx,%cx
[ 	]*[a-f0-9]+:	01 d1                	addl   %edx,%ecx
[ 	]*[a-f0-9]+:	03 ca                	addl.s %edx,%ecx
[ 	]*[a-f0-9]+:	10 d1                	adcb   %dl,%cl
[ 	]*[a-f0-9]+:	12 ca                	adcb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 11 d1             	adcw   %dx,%cx
[ 	]*[a-f0-9]+:	66 13 ca             	adcw.s %dx,%cx
[ 	]*[a-f0-9]+:	11 d1                	adcl   %edx,%ecx
[ 	]*[a-f0-9]+:	13 ca                	adcl.s %edx,%ecx
[ 	]*[a-f0-9]+:	20 d1                	andb   %dl,%cl
[ 	]*[a-f0-9]+:	22 ca                	andb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 21 d1             	andw   %dx,%cx
[ 	]*[a-f0-9]+:	66 23 ca             	andw.s %dx,%cx
[ 	]*[a-f0-9]+:	21 d1                	andl   %edx,%ecx
[ 	]*[a-f0-9]+:	23 ca                	andl.s %edx,%ecx
[ 	]*[a-f0-9]+:	38 d1                	cmpb   %dl,%cl
[ 	]*[a-f0-9]+:	3a ca                	cmpb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 39 d1             	cmpw   %dx,%cx
[ 	]*[a-f0-9]+:	66 3b ca             	cmpw.s %dx,%cx
[ 	]*[a-f0-9]+:	39 d1                	cmpl   %edx,%ecx
[ 	]*[a-f0-9]+:	3b ca                	cmpl.s %edx,%ecx
[ 	]*[a-f0-9]+:	88 d1                	movb   %dl,%cl
[ 	]*[a-f0-9]+:	8a ca                	movb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 89 d1             	movw   %dx,%cx
[ 	]*[a-f0-9]+:	66 8b ca             	movw.s %dx,%cx
[ 	]*[a-f0-9]+:	89 d1                	movl   %edx,%ecx
[ 	]*[a-f0-9]+:	8b ca                	movl.s %edx,%ecx
[ 	]*[a-f0-9]+:	08 d1                	orb    %dl,%cl
[ 	]*[a-f0-9]+:	0a ca                	orb.s  %dl,%cl
[ 	]*[a-f0-9]+:	66 09 d1             	orw    %dx,%cx
[ 	]*[a-f0-9]+:	66 0b ca             	orw.s  %dx,%cx
[ 	]*[a-f0-9]+:	09 d1                	orl    %edx,%ecx
[ 	]*[a-f0-9]+:	0b ca                	orl.s  %edx,%ecx
[ 	]*[a-f0-9]+:	18 d1                	sbbb   %dl,%cl
[ 	]*[a-f0-9]+:	1a ca                	sbbb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 19 d1             	sbbw   %dx,%cx
[ 	]*[a-f0-9]+:	66 1b ca             	sbbw.s %dx,%cx
[ 	]*[a-f0-9]+:	19 d1                	sbbl   %edx,%ecx
[ 	]*[a-f0-9]+:	1b ca                	sbbl.s %edx,%ecx
[ 	]*[a-f0-9]+:	28 d1                	subb   %dl,%cl
[ 	]*[a-f0-9]+:	2a ca                	subb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 29 d1             	subw   %dx,%cx
[ 	]*[a-f0-9]+:	66 2b ca             	subw.s %dx,%cx
[ 	]*[a-f0-9]+:	29 d1                	subl   %edx,%ecx
[ 	]*[a-f0-9]+:	2b ca                	subl.s %edx,%ecx
[ 	]*[a-f0-9]+:	30 d1                	xorb   %dl,%cl
[ 	]*[a-f0-9]+:	32 ca                	xorb.s %dl,%cl
[ 	]*[a-f0-9]+:	66 31 d1             	xorw   %dx,%cx
[ 	]*[a-f0-9]+:	66 33 ca             	xorw.s %dx,%cx
[ 	]*[a-f0-9]+:	31 d1                	xorl   %edx,%ecx
[ 	]*[a-f0-9]+:	33 ca                	xorl.s %edx,%ecx
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 29 e6          	vmovapd.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 29 e6          	vmovaps.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 7f e6          	vmovdqa.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 7f e6          	vmovdqu.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 11 e6          	vmovupd.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 11 e6          	vmovups.s %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 f4          	vmovsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 11 e6          	vmovsd.s %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 10 f4          	vmovss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 11 e6          	vmovss.s %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 e6          	vmovapd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 e6          	vmovaps.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f e6          	vmovdqa.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f e6          	vmovdqu.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 d6 e6          	vmovq.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 e6          	vmovupd.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 e6          	vmovups.s %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 11 e2          	vmovsd.s %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 11 e2          	vmovss.s %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	0f 6f e0             	movq   %mm0,%mm4
[ 	]*[a-f0-9]+:	0f 7f c4             	movq.s %mm0,%mm4
[ 	]*[a-f0-9]+:	66 0f 1a ca          	bndmov %bnd2,%bnd1
[ 	]*[a-f0-9]+:	66 0f 1b d1          	bndmov.s %bnd2,%bnd1
#pass
