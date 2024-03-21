#source: reg.s
#as: -J
#objdump: -dw -Mintel
#name: i386 reg (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 d6 02       	psrlw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 e6 02       	psraw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 f6 02       	psllw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 d6 02       	psrld  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 e6 02       	psrad  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 f6 02       	pslld  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 d6 02       	psrlq  xmm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 de 02       	psrldq xmm6,0x2
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 f6 02       	psllq  xmm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 fe 02       	pslldq xmm6,0x2
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 d6 02       	psrlw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 e6 02       	psraw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 71 f6 02       	psllw  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 d6 02       	psrld  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 e6 02       	psrad  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 72 f6 02       	pslld  xmm6,0x2
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 d6 02       	psrlq  xmm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 de 02       	psrldq xmm6,0x2
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  mm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 f6 02       	psllq  xmm6,0x2
[ 	]*[a-f0-9]+:	66 0f 73 fe 02       	pslldq xmm6,0x2
#pass
