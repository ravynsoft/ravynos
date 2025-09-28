#objdump: -dw
#name: i386 amd

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	0f 0d 03             	prefetch \(%ebx\)
[ 	]*[a-f0-9]+:	0f 0d 0c 75 00 10 00 00 	prefetchw 0x1000\(,%esi,2\)
[ 	]*[a-f0-9]+:	0f 0e                	femms
[ 	]*[a-f0-9]+:	0f 0f 00 bf          	pavgusb \(%eax\),%mm0
[ 	]*[a-f0-9]+:	0f 0f 48 02 1d       	pf2id  0x2\(%eax\),%mm1
[ 	]*[a-f0-9]+:	0f 0f 90 00 01 00 00 ae 	pfacc  0x100\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 0f 1e 9e          	pfadd  \(%esi\),%mm3
[ 	]*[a-f0-9]+:	0f 0f 66 02 b0       	pfcmpeq 0x2\(%esi\),%mm4
[ 	]*[a-f0-9]+:	0f 0f ae 90 90 00 00 90 	pfcmpge 0x9090\(%esi\),%mm5
[ 	]*[a-f0-9]+:	0f 0f 74 75 00 a0    	pfcmpgt 0x0\(%ebp,%esi,2\),%mm6
[ 	]*[a-f0-9]+:	0f 0f 7c 75 02 a4    	pfmax  0x2\(%ebp,%esi,2\),%mm7
[ 	]*[a-f0-9]+:	0f 0f 84 75 90 90 90 90 94 	pfmin  -0x6f6f6f70\(%ebp,%esi,2\),%mm0
[ 	]*[a-f0-9]+:	0f 0f 0d 04 00 00 00 b4 	pfmul  0x4,%mm1
[ 	]*[a-f0-9]+:	2e 0f 0f 54 c3 07 96 	pfrcp  %cs:0x7\(%ebx,%eax,8\),%mm2
[ 	]*[a-f0-9]+:	0f 0f d8 a6          	pfrcpit1 %mm0,%mm3
[ 	]*[a-f0-9]+:	0f 0f e1 b6          	pfrcpit2 %mm1,%mm4
[ 	]*[a-f0-9]+:	0f 0f ea a7          	pfrsqit1 %mm2,%mm5
[ 	]*[a-f0-9]+:	0f 0f f3 97          	pfrsqrt %mm3,%mm6
[ 	]*[a-f0-9]+:	0f 0f fc 9a          	pfsub  %mm4,%mm7
[ 	]*[a-f0-9]+:	0f 0f c5 aa          	pfsubr %mm5,%mm0
[ 	]*[a-f0-9]+:	0f 0f ce 0d          	pi2fd  %mm6,%mm1
[ 	]*[a-f0-9]+:	0f 0f d7 b7          	pmulhrw %mm7,%mm2
[ 	]*[a-f0-9]+:	0f 05                	syscall
[ 	]*[a-f0-9]+:	0f 07                	sysret
[ 	]*[a-f0-9]+:	0f 01 f9             	rdtscp
[ 	]*[a-f0-9]+:	2e 0f                	\(bad\)
[ 	]*[a-f0-9]+:	0f 54 c3             	andps  %xmm3,%xmm0
[ 	]*[a-f0-9]+:	07                   	pop    %es
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
