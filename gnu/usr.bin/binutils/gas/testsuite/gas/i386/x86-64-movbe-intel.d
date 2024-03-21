#objdump: -drwMintel
#name: x86-64 movbe (Intel mode)
#source: x86-64-movbe.s

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbe  r13w,WORD PTR \[r9\]
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbe  r13d,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbe  r13,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbe  WORD PTR \[r9\],r13w
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbe  DWORD PTR \[r9\],r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbe  QWORD PTR \[r9\],r13
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbe  r13w,WORD PTR \[r9\]
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbe  r13d,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbe  r13,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbe  WORD PTR \[r9\],r13w
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbe  DWORD PTR \[r9\],r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbe  QWORD PTR \[r9\],r13
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  bx,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	48 0f 38 f0 19       	movbe  rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  WORD PTR \[rcx\],bx
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	48 0f 38 f1 19       	movbe  QWORD PTR \[rcx\],rbx
#pass
