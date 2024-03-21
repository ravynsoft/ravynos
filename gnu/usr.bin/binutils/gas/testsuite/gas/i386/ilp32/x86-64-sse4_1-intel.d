#source: ../x86-64-sse4_1.s
#objdump: -dwMintel
#name: x86-64 (ILP32) SSE4.1 (Intel disassembly)

.*:     file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	66 0f 3a 0d 01 00    	blendpd xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0d c1 00    	blendpd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0c 01 00    	blendps xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0c c1 00    	blendps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 15 01       	blendvpd xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 15 c1       	blendvpd xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 38 15 01       	blendvpd xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 15 c1       	blendvpd xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 01       	blendvps xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 c1       	blendvps xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 01       	blendvps xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 c1       	blendvps xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 41 01 00    	dppd   xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 41 c1 00    	dppd   xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 40 01 00    	dpps   xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 40 c1 00    	dpps   xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 c1 00    	extractps ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 c1 00    	extractps ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 01 00    	extractps DWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 21 c1 00    	insertps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 21 01 00    	insertps xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 38 2a 01       	movntdqa xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 42 01 00    	mpsadbw xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 42 c1 00    	mpsadbw xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 2b 01       	packusdw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 2b c1       	packusdw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 10 01       	pblendvb xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 10 c1       	pblendvb xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 38 10 01       	pblendvb xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 10 c1       	pblendvb xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0e 01 00    	pblendw xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0e c1 00    	pblendw xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 29 c1       	pcmpeqq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 29 01       	pcmpeqq xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 14 c1 00    	pextrb ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 14 c1 00    	pextrb ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 14 01 00    	pextrb BYTE PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 16 c1 00    	pextrd ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 16 01 00    	pextrd DWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 16 c1 00 	pextrq rcx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 16 01 00 	pextrq QWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f c5 c8 00       	pextrw ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f c5 c8 00       	pextrw ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 15 01 00    	pextrw WORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 38 41 c1       	phminposuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 41 01       	phminposuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 20 01 00    	pinsrb xmm0,BYTE PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 20 c1 00    	pinsrb xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 20 c1 00    	pinsrb xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 22 01 00    	pinsrd xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 22 c1 00    	pinsrd xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 22 01 00 	pinsrq xmm0,QWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 22 c1 00 	pinsrq xmm0,rcx,0x0
[ 	]*[a-f0-9]+:	66 0f 38 3c c1       	pmaxsb xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3c 01       	pmaxsb xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3d c1       	pmaxsd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3d 01       	pmaxsd xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3f c1       	pmaxud xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3f 01       	pmaxud xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3e c1       	pmaxuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3e 01       	pmaxuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 38 c1       	pminsb xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 38 01       	pminsb xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 39 c1       	pminsd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 39 01       	pminsd xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3b c1       	pminud xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3b 01       	pminud xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3a c1       	pminuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3a 01       	pminuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 20 c1       	pmovsxbw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 20 01       	pmovsxbw xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 21 c1       	pmovsxbd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 21 01       	pmovsxbd xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 22 c1       	pmovsxbq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 22 01       	pmovsxbq xmm0,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 23 c1       	pmovsxwd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 23 01       	pmovsxwd xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 24 c1       	pmovsxwq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 24 01       	pmovsxwq xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 25 c1       	pmovsxdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 25 01       	pmovsxdq xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 30 c1       	pmovzxbw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 30 01       	pmovzxbw xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 31 c1       	pmovzxbd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 31 01       	pmovzxbd xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 32 c1       	pmovzxbq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 32 01       	pmovzxbq xmm0,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 33 c1       	pmovzxwd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 33 01       	pmovzxwd xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 34 c1       	pmovzxwq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 34 01       	pmovzxwq xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 35 c1       	pmovzxdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 35 01       	pmovzxdq xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 28 c1       	pmuldq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 28 01       	pmuldq xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 40 c1       	pmulld xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 40 01       	pmulld xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 17 c1       	ptest  xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 17 01       	ptest  xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 09 01 00    	roundpd xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 09 c1 00    	roundpd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 08 01 00    	roundps xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 08 c1 00    	roundps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 01 00    	roundsd xmm0,QWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0b c1 00    	roundsd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 01 00    	roundss xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0a c1 00    	roundss xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0d 01 00    	blendpd xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0d c1 00    	blendpd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0c 01 00    	blendps xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0c c1 00    	blendps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 15 01       	blendvpd xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 15 c1       	blendvpd xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 01       	blendvps xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 14 c1       	blendvps xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 41 01 00    	dppd   xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 41 c1 00    	dppd   xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 40 01 00    	dpps   xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 40 c1 00    	dpps   xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 c1 00    	extractps ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 c1 00    	extractps ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 17 01 00    	extractps DWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 21 c1 00    	insertps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 21 01 00    	insertps xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 38 2a 01       	movntdqa xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 42 01 00    	mpsadbw xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 42 c1 00    	mpsadbw xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 2b 01       	packusdw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 2b c1       	packusdw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 10 01       	pblendvb xmm0,XMMWORD PTR \[rcx\],xmm0
[ 	]*[a-f0-9]+:	66 0f 38 10 c1       	pblendvb xmm0,xmm1,xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0e 01 00    	pblendw xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0e c1 00    	pblendw xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 38 29 c1       	pcmpeqq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 29 01       	pcmpeqq xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 14 c1 00    	pextrb ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 14 c1 00    	pextrb ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 14 01 00    	pextrb BYTE PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 16 c1 00    	pextrd ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 16 01 00    	pextrd DWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 16 c1 00 	pextrq rcx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 16 01 00 	pextrq QWORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f c5 c8 00       	pextrw ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f c5 c8 00       	pextrw ecx,xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 15 01 00    	pextrw WORD PTR \[rcx\],xmm0,0x0
[ 	]*[a-f0-9]+:	66 0f 38 41 c1       	phminposuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 41 01       	phminposuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 20 01 00    	pinsrb xmm0,BYTE PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 20 c1 00    	pinsrb xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 20 c1 00    	pinsrb xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 22 01 00    	pinsrd xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 22 c1 00    	pinsrd xmm0,ecx,0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 22 01 00 	pinsrq xmm0,QWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 48 0f 3a 22 c1 00 	pinsrq xmm0,rcx,0x0
[ 	]*[a-f0-9]+:	66 0f 38 3c c1       	pmaxsb xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3c 01       	pmaxsb xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3d c1       	pmaxsd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3d 01       	pmaxsd xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3f c1       	pmaxud xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3f 01       	pmaxud xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3e c1       	pmaxuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3e 01       	pmaxuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 38 c1       	pminsb xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 38 01       	pminsb xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 39 c1       	pminsd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 39 01       	pminsd xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3b c1       	pminud xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3b 01       	pminud xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 3a c1       	pminuw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 3a 01       	pminuw xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 20 c1       	pmovsxbw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 20 01       	pmovsxbw xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 21 c1       	pmovsxbd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 21 01       	pmovsxbd xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 22 c1       	pmovsxbq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 22 01       	pmovsxbq xmm0,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 23 c1       	pmovsxwd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 23 01       	pmovsxwd xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 24 c1       	pmovsxwq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 24 01       	pmovsxwq xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 25 c1       	pmovsxdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 25 01       	pmovsxdq xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 30 c1       	pmovzxbw xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 30 01       	pmovzxbw xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 31 c1       	pmovzxbd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 31 01       	pmovzxbd xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 32 c1       	pmovzxbq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 32 01       	pmovzxbq xmm0,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 33 c1       	pmovzxwd xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 33 01       	pmovzxwd xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 34 c1       	pmovzxwq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 34 01       	pmovzxwq xmm0,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 35 c1       	pmovzxdq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 35 01       	pmovzxdq xmm0,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 28 c1       	pmuldq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 28 01       	pmuldq xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 40 c1       	pmulld xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 40 01       	pmulld xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 17 c1       	ptest  xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 38 17 01       	ptest  xmm0,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 3a 09 01 00    	roundpd xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 09 c1 00    	roundpd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 08 01 00    	roundps xmm0,XMMWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 08 c1 00    	roundps xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 01 00    	roundsd xmm0,QWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0b c1 00    	roundsd xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 01 00    	roundss xmm0,DWORD PTR \[rcx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 0a c1 00    	roundss xmm0,xmm1,0x0
#pass
