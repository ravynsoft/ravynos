#objdump: -dwMintel
#name: i386 SSE4.2 (Intel disassembly)
#source: sse4_2.s

.*:     file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	f2 0f 38 f0 d9       	crc32  ebx,cl
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 d9    	crc32  ebx,cx
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  ebx,ecx
[ 	]*[a-f0-9]+:	f2 0f 38 f0 19       	crc32  ebx,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 19    	crc32  ebx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f2 0f 38 f1 19       	crc32  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f2 0f 38 f0 d9       	crc32  ebx,cl
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 d9    	crc32  ebx,cx
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  ebx,ecx
[ 	]*[a-f0-9]+:	66 0f 38 37 01       	pcmpgtq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 37 c1       	pcmpgtq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 61 01 00    	pcmpestri xmm0,XMMWORD PTR \[ecx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 61 c1 00    	pcmpestri xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 60 01 01    	pcmpestrm xmm0,XMMWORD PTR \[ecx\],0x1
[ 	]*[a-f0-9]+:	66 0f 3a 60 c1 01    	pcmpestrm xmm0,xmm1,0x1
[ 	]*[a-f0-9]+:	66 0f 3a 63 01 02    	pcmpistri xmm0,XMMWORD PTR \[ecx\],0x2
[ 	]*[a-f0-9]+:	66 0f 3a 63 c1 02    	pcmpistri xmm0,xmm1,0x2
[ 	]*[a-f0-9]+:	66 0f 3a 62 01 03    	pcmpistrm xmm0,XMMWORD PTR \[ecx\],0x3
[ 	]*[a-f0-9]+:	66 0f 3a 62 c1 03    	pcmpistrm xmm0,xmm1,0x3
[ 	]*[a-f0-9]+:	66 f3 0f b8 19       	popcnt bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f3 0f b8 19          	popcnt ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f b8 19       	popcnt bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f3 0f b8 19          	popcnt ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f b8 d9       	popcnt bx,cx
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt ebx,ecx
[ 	]*[a-f0-9]+:	66 f3 0f b8 d9       	popcnt bx,cx
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt ebx,ecx
[ 	]*[a-f0-9]+:	f2 0f 38 f0 d9       	crc32  ebx,cl
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 d9    	crc32  ebx,cx
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  ebx,ecx
[ 	]*[a-f0-9]+:	f2 0f 38 f0 19       	crc32  ebx,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 19    	crc32  ebx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f2 0f 38 f1 19       	crc32  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f2 0f 38 f0 d9       	crc32  ebx,cl
[ 	]*[a-f0-9]+:	66 f2 0f 38 f1 d9    	crc32  ebx,cx
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  ebx,ecx
[ 	]*[a-f0-9]+:	66 0f 38 37 01       	pcmpgtq xmm0,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 37 c1       	pcmpgtq xmm0,xmm1
[ 	]*[a-f0-9]+:	66 0f 3a 61 01 00    	pcmpestri xmm0,XMMWORD PTR \[ecx\],0x0
[ 	]*[a-f0-9]+:	66 0f 3a 61 c1 00    	pcmpestri xmm0,xmm1,0x0
[ 	]*[a-f0-9]+:	66 0f 3a 60 01 01    	pcmpestrm xmm0,XMMWORD PTR \[ecx\],0x1
[ 	]*[a-f0-9]+:	66 0f 3a 60 c1 01    	pcmpestrm xmm0,xmm1,0x1
[ 	]*[a-f0-9]+:	66 0f 3a 63 01 02    	pcmpistri xmm0,XMMWORD PTR \[ecx\],0x2
[ 	]*[a-f0-9]+:	66 0f 3a 63 c1 02    	pcmpistri xmm0,xmm1,0x2
[ 	]*[a-f0-9]+:	66 0f 3a 62 01 03    	pcmpistrm xmm0,XMMWORD PTR \[ecx\],0x3
[ 	]*[a-f0-9]+:	66 0f 3a 62 c1 03    	pcmpistrm xmm0,xmm1,0x3
[ 	]*[a-f0-9]+:	66 f3 0f b8 19       	popcnt bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f3 0f b8 19          	popcnt ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f b8 19       	popcnt bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f3 0f b8 19          	popcnt ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f b8 d9       	popcnt bx,cx
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt ebx,ecx
[ 	]*[a-f0-9]+:	66 f3 0f b8 d9       	popcnt bx,cx
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt ebx,ecx
[ 	]*[a-f0-9]+:	00 00                	add    BYTE PTR \[eax\],al
#pass
