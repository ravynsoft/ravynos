#source: x86-64-reg.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 reg (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 71 d2 02    	psrlw  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 71 e2 02    	psraw  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 71 f2 02    	psllw  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 72 d2 02    	psrld  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 72 e2 02    	psrad  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 72 f2 02    	pslld  xmm10,0x2
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 73 d2 02    	psrlq  xmm10,0x2
[ 	]*[a-f0-9]+:	66 41 0f 73 da 02    	psrldq xmm10,0x2
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  mm6,0x2
[ 	]*[a-f0-9]+:	66 41 0f 73 f2 02    	psllq  xmm10,0x2
[ 	]*[a-f0-9]+:	66 41 0f 73 fa 02    	pslldq xmm10,0x2
[ 	]*[a-f0-9]+:	40 80 c0 01[ 	]+rex add al,0x1
[ 	]*[a-f0-9]+:	40 80 c1 01[ 	]+rex add cl,0x1
[ 	]*[a-f0-9]+:	40 80 c2 01[ 	]+rex add dl,0x1
[ 	]*[a-f0-9]+:	40 80 c3 01[ 	]+rex add bl,0x1
[ 	]*[a-f0-9]+:	40 80 c4 01[ 	]+add    spl,0x1
[ 	]*[a-f0-9]+:	40 80 c5 01[ 	]+add    bpl,0x1
[ 	]*[a-f0-9]+:	40 80 c6 01[ 	]+add    sil,0x1
[ 	]*[a-f0-9]+:	40 80 c7 01[ 	]+add    dil,0x1
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 d2 02       	psrlw  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 e2 02       	psraw  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 f2 02       	psllw  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 d2 02       	psrld  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 e2 02       	psrad  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 f2 02       	pslld  xmm2,0x2
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 d2 02       	psrlq  xmm2,0x2
[ 	]*[a-f0-9]+:	66 0f 73 da 02       	psrldq xmm2,0x2
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 f2 02       	psllq  xmm2,0x2
[ 	]*[a-f0-9]+:	66 0f 73 fa 02       	pslldq xmm2,0x2
#pass
