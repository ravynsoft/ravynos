#objdump: -dw
#name: i386 arch 1

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	66 0f 38 17 c1       	ptest  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 09 c1 00    	roundpd \$0x0,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 08 c1 00    	roundps \$0x0,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0b c1 00    	roundsd \$0x0,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0a c1 00    	roundss \$0x0,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 41 d9       	phminposuw %xmm1,%xmm3
#pass
