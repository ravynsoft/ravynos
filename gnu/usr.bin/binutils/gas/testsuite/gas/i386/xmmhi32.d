#objdump: -dwr
#name: high/disabled XMM/mask registers in 32-bit mode

.*: +file format .*

Disassembly of section .text:

0+ <xmm>:
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm8
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm16
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm24
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm8
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm16
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm24
[ 	]*[a-f0-9]+:	62 f1 74 48 58 05 00 00 00 00 	vaddps 0x0,%zmm1,%zmm0	[a-f0-9]+: (R_386_|dir)?32	zmm8
[ 	]*[a-f0-9]+:	62 f1 74 48 58 05 00 00 00 00 	vaddps 0x0,%zmm1,%zmm0	[a-f0-9]+: (R_386_|dir)?32	zmm16
[ 	]*[a-f0-9]+:	62 f1 74 48 58 05 00 00 00 00 	vaddps 0x0,%zmm1,%zmm0	[a-f0-9]+: (R_386_|dir)?32	zmm24
[ 	]*[a-f0-9]+:	c5 f9 6f 05 00 00 00 00 	vmovdqa 0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm8
[ 	]*[a-f0-9]+:	c5 f9 6f 05 00 00 00 00 	vmovdqa 0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm16
[ 	]*[a-f0-9]+:	c5 f9 6f 05 00 00 00 00 	vmovdqa 0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm24
[ 	]*[a-f0-9]+:	c5 fd 6f 05 00 00 00 00 	vmovdqa 0x0,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm8
[ 	]*[a-f0-9]+:	c5 fd 6f 05 00 00 00 00 	vmovdqa 0x0,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm16
[ 	]*[a-f0-9]+:	c5 fd 6f 05 00 00 00 00 	vmovdqa 0x0,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm24
[ 	]*[a-f0-9]+:	c5 f9 7f 05 00 00 00 00 	vmovdqa %xmm0,0x0	[a-f0-9]+: (R_386_|dir)?32	xmm8
[ 	]*[a-f0-9]+:	c5 f9 7f 05 00 00 00 00 	vmovdqa %xmm0,0x0	[a-f0-9]+: (R_386_|dir)?32	xmm16
[ 	]*[a-f0-9]+:	c5 f9 7f 05 00 00 00 00 	vmovdqa %xmm0,0x0	[a-f0-9]+: (R_386_|dir)?32	xmm24
[ 	]*[a-f0-9]+:	c5 fd 7f 05 00 00 00 00 	vmovdqa %ymm0,0x0	[a-f0-9]+: (R_386_|dir)?32	ymm8
[ 	]*[a-f0-9]+:	c5 fd 7f 05 00 00 00 00 	vmovdqa %ymm0,0x0	[a-f0-9]+: (R_386_|dir)?32	ymm16
[ 	]*[a-f0-9]+:	c5 fd 7f 05 00 00 00 00 	vmovdqa %ymm0,0x0	[a-f0-9]+: (R_386_|dir)?32	ymm24
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm8
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm16
[ 	]*[a-f0-9]+:	c5 f0 58 05 00 00 00 00 	vaddps 0x0,%xmm1,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm24
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm8
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm16
[ 	]*[a-f0-9]+:	c5 f4 58 05 00 00 00 00 	vaddps 0x0,%ymm1,%ymm0	[a-f0-9]+: (R_386_|dir)?32	ymm24
[ 	]*[a-f0-9]+:	c5 f9 6f 05 00 00 00 00 	vmovdqa 0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	zmm0
[ 	]*[a-f0-9]+:	c5 f9 6f 05 00 00 00 00 	vmovdqa 0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	k0
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm8
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm16
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	xmm24
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	ymm0
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	ymm8
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	ymm16
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	ymm24
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	zmm0
[ 	]*[a-f0-9]+:	0f 58 05 00 00 00 00 	addps  0x0,%xmm0	[a-f0-9]+: (R_386_|dir)?32	k0
[ 	]*[a-f0-9]+:	a1 00 00 00 00       	mov    0x0,%eax	[a-f0-9]+: (R_386_|dir)?32	xmm0
[ 	]*[a-f0-9]+:	a1 00 00 00 00       	mov    0x0,%eax	[a-f0-9]+: (R_386_|dir)?32	ymm0
[ 	]*[a-f0-9]+:	a1 00 00 00 00       	mov    0x0,%eax	[a-f0-9]+: (R_386_|dir)?32	zmm0
[ 	]*[a-f0-9]+:	a1 00 00 00 00       	mov    0x0,%eax	[a-f0-9]+: (R_386_|dir)?32	k0
#pass
