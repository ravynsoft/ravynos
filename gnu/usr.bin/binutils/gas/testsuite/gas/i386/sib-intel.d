#source: sib.s
#objdump: -dw -Mintel
#name: i386 SIB (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	8b 1d e2 ff ff ff    	mov    ebx,DWORD PTR ds:0xffffffe2
[ 	]*[a-f0-9]+:	8b 1c 25 e2 ff ff ff 	mov    ebx,DWORD PTR \[eiz\*1-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*1-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*2-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*4-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*8-0x1e\]
[ 	]*[a-f0-9]+:	a1 1e 00 00 00       	mov    eax,ds:0x1e
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*1\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*1\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*2\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*4\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*8\+0x1e\]
[ 	]*[a-f0-9]+:	8b 03                	mov    eax,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[ebx\+eiz\*1\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[ebx\+eiz\*1\]
[ 	]*[a-f0-9]+:	8b 04 63             	mov    eax,DWORD PTR \[ebx\+eiz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    eax,DWORD PTR \[ebx\+eiz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    eax,DWORD PTR \[ebx\+eiz\*8\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	8b 04 64             	mov    eax,DWORD PTR \[esp\+eiz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    eax,DWORD PTR \[esp\+eiz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    eax,DWORD PTR \[esp\+eiz\*8\]
[ 	]*[a-f0-9]+:	8b 04 00             	mov    eax,DWORD PTR \[eax\+eax\*1\]
[ 	]*[a-f0-9]+:	8b 04 40             	mov    eax,DWORD PTR \[eax\+eax\*2\]
[ 	]*[a-f0-9]+:	8b 04 80             	mov    eax,DWORD PTR \[eax\+eax\*4\]
[ 	]*[a-f0-9]+:	8b 04 c0             	mov    eax,DWORD PTR \[eax\+eax\*8\]
[ 	]*[a-f0-9]+:	8b 14 08             	mov    edx,DWORD PTR \[eax\+ecx\*1\]
[ 	]*[a-f0-9]+:	8b 14 48             	mov    edx,DWORD PTR \[eax\+ecx\*2\]
[ 	]*[a-f0-9]+:	8b 14 88             	mov    edx,DWORD PTR \[eax\+ecx\*4\]
[ 	]*[a-f0-9]+:	8b 14 c8             	mov    edx,DWORD PTR \[eax\+ecx\*8\]
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*1-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*2-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*4-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    eax,DWORD PTR \[eiz\*8-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*1\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*2\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*4\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    eax,DWORD PTR \[eiz\*8\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[ebx\+eiz\*1\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[ebx\+eiz\*1\]
[ 	]*[a-f0-9]+:	8b 04 63             	mov    eax,DWORD PTR \[ebx\+eiz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    eax,DWORD PTR \[ebx\+eiz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    eax,DWORD PTR \[ebx\+eiz\*8\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	8b 04 64             	mov    eax,DWORD PTR \[esp\+eiz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    eax,DWORD PTR \[esp\+eiz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    eax,DWORD PTR \[esp\+eiz\*8\]
#pass
