#source: x86-64-disp.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 displacement (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	8b 98 ff ff ff 7f    	mov    ebx,DWORD PTR \[rax\+0x7fffffff\]
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    ebx,DWORD PTR \[rax-0x80000000\]
[ 	]*[a-f0-9]+:	8b 1c 25 00 00 00 80 	mov    ebx,DWORD PTR ds:0xffffffff80000000
[ 	]*[a-f0-9]+:	8b 1c 25 00 00 00 80 	mov    ebx,DWORD PTR ds:0xffffffff80000000
[ 	]*[a-f0-9]+:	8b 1c 25 ff ff ff 7f 	mov    ebx,DWORD PTR ds:0x7fffffff
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 80 	mov    eax,DWORD PTR ds:0xffffffff80000000
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 80 	mov    eax,DWORD PTR ds:0xffffffff80000000
[ 	]*[a-f0-9]+:	8b 04 25 ff ff ff 7f 	mov    eax,DWORD PTR ds:0x7fffffff
[ 	]*[a-f0-9]+:	a1 00 00 00 80 00 00 00 00 	movabs eax,ds:0x80000000
[ 	]*[a-f0-9]+:	b8 f0 00 e0 0e       	mov    eax,0xee000f0
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    DWORD PTR \[rax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    DWORD PTR \[rax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    DWORD PTR gs:\[rax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    DWORD PTR gs:\[rax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	89 1c 25 f0 00 e0 0e 	mov    DWORD PTR ds:0xee000f0,ebx
[ 	]*[a-f0-9]+:	65 89 1c 25 f0 00 e0 0e 	mov    DWORD PTR gs:0xee000f0,ebx
[ 	]*[a-f0-9]+:	89 04 25 f0 00 e0 0e 	mov    DWORD PTR ds:0xee000f0,eax
[ 	]*[a-f0-9]+:	65 89 04 25 f0 00 e0 0e 	mov    DWORD PTR gs:0xee000f0,eax
[ 	]*[a-f0-9]+:	a3 f0 00 e0 fe 00 00 00 00 	movabs ds:0xfee000f0,eax
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 fe 00 00 00 00 	movabs gs:0xfee000f0,eax
[ 	]*[a-f0-9]+:	65 8b 1c 25 f0 00 e0 0e 	mov    ebx,DWORD PTR gs:0xee000f0
[ 	]*[a-f0-9]+:	8b 1c 25 f0 00 e0 0e 	mov    ebx,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	8b 1c 25 f0 00 e0 0e 	mov    ebx,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	65 8b 04 25 f0 00 e0 0e 	mov    eax,DWORD PTR gs:0xee000f0
[ 	]*[a-f0-9]+:	8b 04 25 f0 00 e0 0e 	mov    eax,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	8b 04 25 f0 00 e0 0e 	mov    eax,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 fe 00 00 00 00 	movabs eax,gs:0xfee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe 00 00 00 00 	movabs eax,ds:0xfee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe 00 00 00 00 	movabs eax,ds:0xfee000f0
#pass
