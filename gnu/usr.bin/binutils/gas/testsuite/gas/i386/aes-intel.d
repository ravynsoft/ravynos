#source: aes.s
#objdump: -dw -Mintel
#name: i386 AES (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist xmm0,XMMWORD PTR \[ecx\],0x8
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist xmm0,xmm1,0x8
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist xmm0,XMMWORD PTR \[ecx\],0x8
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist xmm0,xmm1,0x8
#pass
