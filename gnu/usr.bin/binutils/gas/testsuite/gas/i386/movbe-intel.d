#objdump: -dwMintel
#name: i386 movbe (Intel disassembly)
#source: movbe.s

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  WORD PTR \[ecx\],bx
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  WORD PTR \[ecx\],bx
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  WORD PTR \[ecx\],bx
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  DWORD PTR \[ecx\],ebx
#pass
