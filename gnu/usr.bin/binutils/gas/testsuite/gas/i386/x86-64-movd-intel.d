#source: x86-64-movd.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 movd (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	66 0f 6e 88 80 00 00 00 	movd   xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	66 48 0f 6e c8       	movq   xmm1,rax
 +[a-f0-9]+:	66 0f 7e 88 80 00 00 00 	movd   DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	66 48 0f 7e c8       	movq   rax,xmm1
 +[a-f0-9]+:	c5 f9 6e 88 80 00 00 00 	vmovd  xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	c4 e1 f9 6e c8       	vmovq  xmm1,rax
 +[a-f0-9]+:	c5 f9 7e 88 80 00 00 00 	vmovd  DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	c4 e1 f9 7e c8       	vmovq  rax,xmm1
 +[a-f0-9]+:	62 f1 7d 08 6e 48 20 	\{evex\} vmovd xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	62 f1 7d 08 7e 48 20 	\{evex\} vmovd DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	66 0f 6e 88 80 00 00 00 	movd   xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	66 0f 6e 88 80 00 00 00 	movd   xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	66 0f 6e c8          	movd   xmm1,eax
 +[a-f0-9]+:	66 0f 7e 88 80 00 00 00 	movd   DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	66 0f 7e 88 80 00 00 00 	movd   DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	66 0f 7e c8          	movd   eax,xmm1
 +[a-f0-9]+:	66 48 0f 6e 88 80 00 00 00 	movq   xmm1,QWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	66 48 0f 6e c8       	movq   xmm1,rax
 +[a-f0-9]+:	66 48 0f 7e 88 80 00 00 00 	movq   QWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	66 48 0f 7e c8       	movq   rax,xmm1
 +[a-f0-9]+:	c5 f9 6e 88 80 00 00 00 	vmovd  xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	c5 f9 6e 88 80 00 00 00 	vmovd  xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	c5 f9 6e c8          	vmovd  xmm1,eax
 +[a-f0-9]+:	c5 f9 7e 88 80 00 00 00 	vmovd  DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	c5 f9 7e 88 80 00 00 00 	vmovd  DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	c5 f9 7e c8          	vmovd  eax,xmm1
 +[a-f0-9]+:	62 f1 7d 08 6e 48 20 	\{evex\} vmovd xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	62 f1 7d 08 6e 48 20 	\{evex\} vmovd xmm1,DWORD PTR \[rax\+0x80\]
 +[a-f0-9]+:	62 f1 7d 08 6e c8    	\{evex\} vmovd xmm1,eax
 +[a-f0-9]+:	62 f1 7d 08 7e 48 20 	\{evex\} vmovd DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	62 f1 7d 08 7e 48 20 	\{evex\} vmovd DWORD PTR \[rax\+0x80\],xmm1
 +[a-f0-9]+:	62 f1 7d 08 7e c8    	\{evex\} vmovd eax,xmm1
 +[a-f0-9]+:	c4 e1 f9 6e c8       	vmovq  xmm1,rax
 +[a-f0-9]+:	c4 e1 f9 7e c8       	vmovq  rax,xmm1
#pass
