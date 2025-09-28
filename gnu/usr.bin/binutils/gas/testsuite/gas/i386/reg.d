#objdump: -dw
#name: i386 reg

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 d6 02       	psrlw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 e6 02       	psraw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 f6 02       	psllw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 d6 02       	psrld  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 e6 02       	psrad  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 f6 02       	pslld  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 d6 02       	psrlq  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	66 0f 73 de 02       	psrldq \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 f6 02       	psllq  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	66 0f 73 fe 02       	pslldq \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 71 d6 02          	psrlw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 d6 02       	psrlw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 71 e6 02          	psraw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 e6 02       	psraw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 71 f6 02          	psllw  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 71 f6 02       	psllw  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 d6 02          	psrld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 d6 02       	psrld  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 e6 02          	psrad  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 e6 02       	psrad  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 72 f6 02          	pslld  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 72 f6 02       	pslld  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 73 d6 02          	psrlq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 d6 02       	psrlq  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	66 0f 73 de 02       	psrldq \$0x2,%xmm6
[ 	]*[a-f0-9]+:	0f 73 f6 02          	psllq  \$0x2,%mm6
[ 	]*[a-f0-9]+:	66 0f 73 f6 02       	psllq  \$0x2,%xmm6
[ 	]*[a-f0-9]+:	66 0f 73 fe 02       	pslldq \$0x2,%xmm6
#pass
