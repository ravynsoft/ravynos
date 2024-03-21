#source: clmul.s
#objdump: -dw -Mintel
#name: i386 PCLMUL (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq xmm0,XMMWORD PTR \[ecx\],0x8
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq xmm0,xmm1,0x8
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq xmm0,XMMWORD PTR \[ecx\],0x8
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq xmm0,xmm1,0x8
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq xmm0,xmm1
#pass
