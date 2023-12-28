#source: ../x86-64-sib.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 (ILP32) SIB (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	8b 1c 25 e2 ff ff ff 	mov    ebx,DWORD PTR ds:0xffffffffffffffe2
[ 	]*[a-f0-9]+:	8b 1c 25 e2 ff ff ff 	mov    ebx,DWORD PTR ds:0xffffffffffffffe2
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    eax,DWORD PTR ds:0xffffffffffffffe2
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*2-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*4-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*8-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR ds:0x1e
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR ds:0x1e
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR ds:0x1e
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*2\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*4\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*8\+0x1e\]
[ 	]*[a-f0-9]+:	8b 03                	mov    eax,DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[rbx\+riz\*1\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[rbx\+riz\*1\]
[ 	]*[a-f0-9]+:	8b 04 63             	mov    eax,DWORD PTR \[rbx\+riz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    eax,DWORD PTR \[rbx\+riz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    eax,DWORD PTR \[rbx\+riz\*8\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 64             	mov    eax,DWORD PTR \[rsp\+riz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    eax,DWORD PTR \[rsp\+riz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    eax,DWORD PTR \[rsp\+riz\*8\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 64          	mov    eax,DWORD PTR \[r12\+riz\*2\]
[ 	]*[a-f0-9]+:	41 8b 04 a4          	mov    eax,DWORD PTR \[r12\+riz\*4\]
[ 	]*[a-f0-9]+:	41 8b 04 e4          	mov    eax,DWORD PTR \[r12\+riz\*8\]
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    eax,DWORD PTR ds:0xffffffffffffffe2
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*2-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*4-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    eax,DWORD PTR \[riz\*8-0x1e\]
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    eax,DWORD PTR ds:0x1e
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*2\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*4\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    eax,DWORD PTR \[riz\*8\+0x1e\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[rbx\+riz\*1\]
[ 	]*[a-f0-9]+:	8b 04 23             	mov    eax,DWORD PTR \[rbx\+riz\*1\]
[ 	]*[a-f0-9]+:	8b 04 63             	mov    eax,DWORD PTR \[rbx\+riz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    eax,DWORD PTR \[rbx\+riz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    eax,DWORD PTR \[rbx\+riz\*8\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 24             	mov    eax,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	8b 04 64             	mov    eax,DWORD PTR \[rsp\+riz\*2\]
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    eax,DWORD PTR \[rsp\+riz\*4\]
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    eax,DWORD PTR \[rsp\+riz\*8\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    eax,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	41 8b 04 64          	mov    eax,DWORD PTR \[r12\+riz\*2\]
[ 	]*[a-f0-9]+:	41 8b 04 a4          	mov    eax,DWORD PTR \[r12\+riz\*4\]
[ 	]*[a-f0-9]+:	41 8b 04 e4          	mov    eax,DWORD PTR \[r12\+riz\*8\]
#pass
