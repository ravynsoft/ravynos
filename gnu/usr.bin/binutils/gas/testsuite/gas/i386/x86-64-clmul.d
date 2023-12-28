#as: -J
#objdump: -dw
#name: x86-64 PCLMUL

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq \$0x8,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 08    	pclmulqdq \$0x8,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 00    	pclmullqlqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 00    	pclmullqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 01    	pclmulhqlqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 01    	pclmulhqlqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 10    	pclmullqhqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 10    	pclmullqhqdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 01 11    	pclmulhqhqdq \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 11    	pclmulhqhqdq %xmm1,%xmm0
#pass
