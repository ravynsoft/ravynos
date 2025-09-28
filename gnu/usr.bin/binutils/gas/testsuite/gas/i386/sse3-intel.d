#objdump: -dwMintel
#name: i386 SSE3 (Intel disassembly)
#source: sse3.s

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[0-9a-f]+:	66 0f d0 01[ 	]+addsubpd xmm0,(XMMWORD PTR )?\[ecx\]
[ 	]*[0-9a-f]+:	66 0f d0 ca[ 	]+addsubpd xmm1,xmm2
[ 	]*[0-9a-f]+:	f2 0f d0 13[ 	]+addsubps xmm2,(XMMWORD PTR )?\[ebx\]
[ 	]*[0-9a-f]+:	f2 0f d0 dc[ 	]+addsubps xmm3,xmm4
[ 	]*[0-9a-f]+:	df 88 90 90 90 90[ 	]+fisttp WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[0-9a-f]+:	db 88 90 90 90 90[ 	]+fisttp DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[0-9a-f]+:	dd 88 90 90 90 90[ 	]+fisttp QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[0-9a-f]+:	dd 88 90 90 90 90[ 	]+fisttp QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[0-9a-f]+:	66 0f 7c 65 00[ 	]+haddpd xmm4,(XMMWORD PTR )?\[ebp(\+0x0)\]
[ 	]*[0-9a-f]+:	66 0f 7c ee[ 	]+haddpd xmm5,xmm6
[ 	]*[0-9a-f]+:	f2 0f 7c 37[ 	]+haddps xmm6,(XMMWORD PTR )?\[edi\]
[ 	]*[0-9a-f]+:	f2 0f 7c f8[ 	]+haddps xmm7,xmm0
[ 	]*[0-9a-f]+:	66 0f 7d c1[ 	]+hsubpd xmm0,xmm1
[ 	]*[0-9a-f]+:	66 0f 7d 0a[ 	]+hsubpd xmm1,(XMMWORD PTR )?\[edx\]
[ 	]*[0-9a-f]+:	f2 0f 7d d2[ 	]+hsubps xmm2,xmm2
[ 	]*[0-9a-f]+:	f2 0f 7d 1c 24[ 	]+hsubps xmm3,(XMMWORD PTR )?\[esp\]
[ 	]*[0-9a-f]+:	f2 0f f0 2e[ 	]+lddqu  xmm5,(XMMWORD PTR )?\[esi\]
[ 	]*[0-9a-f]+:	0f 01 c8[ 	]+monitor
[ 	]*[0-9a-f]+:	0f 01 c8[ 	]+monitor
[ 	]*[0-9a-f]+:	f2 0f 12 f7[ 	]+movddup xmm6,xmm7
[ 	]*[0-9a-f]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[eax\]
[ 	]*[0-9a-f]+:	f3 0f 16 01[ 	]+movshdup xmm0,(XMMWORD PTR )?\[ecx\]
[ 	]*[0-9a-f]+:	f3 0f 16 ca[ 	]+movshdup xmm1,xmm2
[ 	]*[0-9a-f]+:	f3 0f 12 13[ 	]+movsldup xmm2,(XMMWORD PTR )?\[ebx\]
[ 	]*[0-9a-f]+:	f3 0f 12 dc[ 	]+movsldup xmm3,xmm4
[ 	]*[0-9a-f]+:	0f 01 c9[ 	]+mwait
[ 	]*[0-9a-f]+:	0f 01 c9[ 	]+mwait
[ 	]*[0-9a-f]+:	67 0f 01 c8[ 	]+addr16 monitor
[ 	]*[0-9a-f]+:	67 0f 01 c8[ 	]+addr16 monitor
[ 	]*[0-9a-f]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[eax\]
[ 	]*[0-9a-f]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[eax\]
[ 	]*[0-9a-f]+:	0f 01 c8[ 	]+monitor
[ 	]*[0-9a-f]+:	67 0f 01 c8[ 	]+addr16 monitor
[ 	]*[0-9a-f]+:	0f 01 c9[ 	]+mwait
#pass
