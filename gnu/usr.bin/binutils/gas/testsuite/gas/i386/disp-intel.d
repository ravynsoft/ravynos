#source: disp.s
#objdump: -dw -Mintel
#name: i386 displacement (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	8b 98 ff ff ff 7f    	mov    ebx,DWORD PTR \[eax\+0x7fffffff\]
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    ebx,DWORD PTR \[eax-0x80000000\]
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    ebx,DWORD PTR \[eax-0x80000000\]
[ 	]*[a-f0-9]+:	8b 1d ff ff ff 7f    	mov    ebx,DWORD PTR ds:0x7fffffff
[ 	]*[a-f0-9]+:	8b 1d 00 00 00 80    	mov    ebx,DWORD PTR ds:0x80000000
[ 	]*[a-f0-9]+:	8b 1d 00 00 00 80    	mov    ebx,DWORD PTR ds:0x80000000
[ 	]*[a-f0-9]+:	a1 ff ff ff 7f       	mov    eax,ds:0x7fffffff
[ 	]*[a-f0-9]+:	a1 00 00 00 80       	mov    eax,ds:0x80000000
[ 	]*[a-f0-9]+:	a1 00 00 00 80       	mov    eax,ds:0x80000000
[ 	]*[a-f0-9]+:	b8 f0 00 e0 0e       	mov    eax,0xee000f0
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    DWORD PTR \[eax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    DWORD PTR \[eax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    DWORD PTR gs:\[eax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    DWORD PTR gs:\[eax\+0xee000f0\],ebx
[ 	]*[a-f0-9]+:	89 1d f0 00 e0 0e    	mov    DWORD PTR ds:0xee000f0,ebx
[ 	]*[a-f0-9]+:	65 89 1d f0 00 e0 0e 	mov    DWORD PTR gs:0xee000f0,ebx
[ 	]*[a-f0-9]+:	89 1d f0 00 e0 fe    	mov    DWORD PTR ds:0xfee000f0,ebx
[ 	]*[a-f0-9]+:	65 89 1d f0 00 e0 fe 	mov    DWORD PTR gs:0xfee000f0,ebx
[ 	]*[a-f0-9]+:	a3 f0 00 e0 0e       	mov    ds:0xee000f0,eax
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 0e    	mov    gs:0xee000f0,eax
[ 	]*[a-f0-9]+:	a3 f0 00 e0 fe       	mov    ds:0xfee000f0,eax
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 fe    	mov    gs:0xfee000f0,eax
[ 	]*[a-f0-9]+:	65 8b 1d f0 00 e0 0e 	mov    ebx,DWORD PTR gs:0xee000f0
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 0e    	mov    ebx,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 0e    	mov    ebx,DWORD PTR ds:0xee000f0
[ 	]*[a-f0-9]+:	65 8b 1d f0 00 e0 fe 	mov    ebx,DWORD PTR gs:0xfee000f0
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 fe    	mov    ebx,DWORD PTR ds:0xfee000f0
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 fe    	mov    ebx,DWORD PTR ds:0xfee000f0
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 0e    	mov    eax,gs:0xee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 0e       	mov    eax,ds:0xee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 0e       	mov    eax,ds:0xee000f0
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 fe    	mov    eax,gs:0xfee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe       	mov    eax,ds:0xfee000f0
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe       	mov    eax,ds:0xfee000f0
#pass
