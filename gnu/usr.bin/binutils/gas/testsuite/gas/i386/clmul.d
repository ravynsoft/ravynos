#objdump: -dw
#name: i386 PCLMUL

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq \$0x8,\(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq \$0x8,\(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq %xmm1,%xmm0
#pass
