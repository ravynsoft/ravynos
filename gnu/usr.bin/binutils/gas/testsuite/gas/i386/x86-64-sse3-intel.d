#objdump: -dwMintel
#name: x86-64 SSE3 (Intel disassembly)
#source: x86-64-sse3.s

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	66 0f d0 01[ 	]+addsubpd xmm0,(XMMWORD PTR )?\[rcx\]
[ 	]*[a-f0-9]+:	66 0f d0 ca[ 	]+addsubpd xmm1,xmm2
[ 	]*[a-f0-9]+:	f2 0f d0 13[ 	]+addsubps xmm2,(XMMWORD PTR )?\[rbx\]
[ 	]*[a-f0-9]+:	f2 0f d0 dc[ 	]+addsubps xmm3,xmm4
[ 	]*[a-f0-9]+:	df 88 90 90 90 00[ 	]+fisttp WORD PTR \[rax\+0x909090\]
[ 	]*[a-f0-9]+:	db 88 90 90 90 00[ 	]+fisttp DWORD PTR \[rax\+0x909090\]
[ 	]*[a-f0-9]+:	dd 88 90 90 90 00[ 	]+fisttp QWORD PTR \[rax\+0x909090\]
[ 	]*[a-f0-9]+:	dd 88 90 90 90 00[ 	]+fisttp QWORD PTR \[rax\+0x909090\]
[ 	]*[a-f0-9]+:	66 0f 7c 65 00[ 	]+haddpd xmm4,(XMMWORD PTR )?\[rbp(\+0x0)\]
[ 	]*[a-f0-9]+:	66 0f 7c ee[ 	]+haddpd xmm5,xmm6
[ 	]*[a-f0-9]+:	f2 0f 7c 37[ 	]+haddps xmm6,(XMMWORD PTR )?\[rdi\]
[ 	]*[a-f0-9]+:	f2 0f 7c f8[ 	]+haddps xmm7,xmm0
[ 	]*[a-f0-9]+:	66 0f 7d c1[ 	]+hsubpd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 7d 0a[ 	]+hsubpd xmm1,(XMMWORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	f2 0f 7d d2[ 	]+hsubps xmm2,xmm2
[ 	]*[a-f0-9]+:	f2 0f 7d 1c 24[ 	]+hsubps xmm3,(XMMWORD PTR )?\[rsp\]
[ 	]*[a-f0-9]+:	f2 0f f0 2e[ 	]+lddqu  xmm5,(XMMWORD PTR )?\[rsi\]
[ 	]*[a-f0-9]+:	0f 01 c8[ 	]+monitor
[ 	]*[a-f0-9]+:	0f 01 c8[ 	]+monitor
[ 	]*[a-f0-9]+:	0f 01 c8[ 	]+monitor
[ 	]*[a-f0-9]+:	f2 0f 12 f7[ 	]+movddup xmm6,xmm7
[ 	]*[a-f0-9]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[rax\]
[ 	]*[a-f0-9]+:	f3 0f 16 01[ 	]+movshdup xmm0,(XMMWORD PTR )?\[rcx\]
[ 	]*[a-f0-9]+:	f3 0f 16 ca[ 	]+movshdup xmm1,xmm2
[ 	]*[a-f0-9]+:	f3 0f 12 13[ 	]+movsldup xmm2,(XMMWORD PTR )?\[rbx\]
[ 	]*[a-f0-9]+:	f3 0f 12 dc[ 	]+movsldup xmm3,xmm4
[ 	]*[a-f0-9]+:	0f 01 c9[ 	]+mwait
[ 	]*[a-f0-9]+:	0f 01 c9[ 	]+mwait
[ 	]*[a-f0-9]+:	0f 01 c9[ 	]+mwait
[ 	]*[a-f0-9]+:	67 0f 01 c8[ 	]+addr32 monitor
[ 	]*[a-f0-9]+:	67 0f 01 c8[ 	]+addr32 monitor
[ 	]*[a-f0-9]+:	67 0f 01 c8[ 	]+addr32 monitor
[ 	]*[a-f0-9]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[rax\]
[ 	]*[a-f0-9]+:	f2 0f 12 38[ 	]+movddup xmm7,(QWORD PTR )?\[rax\]
[ 	]*[a-f0-9]+:	0f 01 c8[ 	]+monitor
[ 	]*[a-f0-9]+:	67 0f 01 c8[ 	]+addr32 monitor
[ 	]*[a-f0-9]+:	0f 01 c9[ 	]+mwait
#pass
