#as: -march=generic64+avx+vmx+smx+xsave+xsaveopt+aes+pclmul+fma+movbe+cx16+lahf_sahf+ept+clflush+syscall+rdtscp+3dnowa+sse4a+svme+abm+padlock+bmi+tbm
#objdump: -dw
#name: x86-64 arch 2

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f 44 d8             	cmove  %eax,%ebx
[ 	]*[a-f0-9]+:	0f ae 38             	clflush \(%rax\)
[ 	]*[a-f0-9]+:	0f 05                	syscall
[ 	]*[a-f0-9]+:	0f fc dc             	paddb  %mm4,%mm3
[ 	]*[a-f0-9]+:	f3 0f 58 dc          	addss  %xmm4,%xmm3
[ 	]*[a-f0-9]+:	f2 0f 58 dc          	addsd  %xmm4,%xmm3
[ 	]*[a-f0-9]+:	66 0f d0 dc          	addsubpd %xmm4,%xmm3
[ 	]*[a-f0-9]+:	66 0f 38 01 dc       	phaddw %xmm4,%xmm3
[ 	]*[a-f0-9]+:	66 0f 38 41 d9       	phminposuw %xmm1,%xmm3
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  %ecx,%ebx
[ 	]*[a-f0-9]+:	c5 fc 77             	vzeroall
[ 	]*[a-f0-9]+:	0f 01 c4             	vmxoff
[ 	]*[a-f0-9]+:	0f 37                	getsec
[ 	]*[a-f0-9]+:	0f 01 d0             	xgetbv
[ 	]*[a-f0-9]+:	0f ae 31             	xsaveopt \(%rcx\)
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 c1 08    	pclmulqdq \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	c4 e2 79 dc 11       	vaesenc \(%rcx\),%xmm0,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 08    	vpclmulqdq \$0x8,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 c9 98 d4       	vfmadd132pd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	48 0f c7 0e          	cmpxchg16b \(%rsi\)
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	0f 01 f9             	rdtscp
[ 	]*[a-f0-9]+:	0f 0d 0c 75 00 10 00 00 	prefetchw 0x1000\(,%rsi,2\)
[ 	]*[a-f0-9]+:	f2 0f 79 ca          	insertq %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 01 da             	vmload
[ 	]*[a-f0-9]+:	f3 0f bd d9          	lzcnt  %ecx,%ebx
[ 	]*[a-f0-9]+:	0f a7 c0             	xstore-rng
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c9       	blsr   %ecx,%ebx
[ 	]*[a-f0-9]+:	8f e9 60 01 c9       	blcfill %ecx,%ebx
[ 	]*[a-f0-9]+:	9f                   	lahf
#pass
