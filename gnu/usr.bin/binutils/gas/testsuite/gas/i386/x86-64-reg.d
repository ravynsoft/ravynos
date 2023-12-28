#as: -J
#objdump: -dw
#name: x86-64 reg

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 71 d2 02    	psrlw  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 71 e2 02    	psraw  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 71 f2 02    	psllw  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 72 d2 02    	psrld  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 72 e2 02    	psrad  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 72 f2 02    	pslld  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 73 d2 02    	psrlq  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	66 41 0f 73 da 02    	psrldq \$0x2,%xmm10
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 41 0f 73 f2 02    	psllq  \$0x2,%xmm10
[ 	]*[a-f0-9]+:	66 41 0f 73 fa 02    	pslldq \$0x2,%xmm10
[ 	]*[a-f0-9]+:	40 80 c0 01[ 	]+rex add \$0x1,%al
[ 	]*[a-f0-9]+:	40 80 c1 01[ 	]+rex add \$0x1,%cl
[ 	]*[a-f0-9]+:	40 80 c2 01[ 	]+rex add \$0x1,%dl
[ 	]*[a-f0-9]+:	40 80 c3 01[ 	]+rex add \$0x1,%bl
[ 	]*[a-f0-9]+:	40 80 c4 01[ 	]+add    \$0x1,%spl
[ 	]*[a-f0-9]+:	40 80 c5 01[ 	]+add    \$0x1,%bpl
[ 	]*[a-f0-9]+:	40 80 c6 01[ 	]+add    \$0x1,%sil
[ 	]*[a-f0-9]+:	40 80 c7 01[ 	]+add    \$0x1,%dil
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 d2 02       	psrlw  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 e2 02       	psraw  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 f2 02       	psllw  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 d2 02       	psrld  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 e2 02       	psrad  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 f2 02       	pslld  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 d2 02       	psrlq  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	66 0f 73 da 02       	psrldq \$0x2,%xmm2
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 f2 02       	psllq  \$0x2,%xmm2
[ 	]*[a-f0-9]+:	66 0f 73 fa 02       	pslldq \$0x2,%xmm2
#pass
