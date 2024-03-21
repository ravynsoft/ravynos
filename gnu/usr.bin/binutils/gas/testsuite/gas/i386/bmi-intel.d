#as:
#objdump: -dwMintel
#name: i386 BMI insns (Intel disassembly)
#source: bmi.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  bx,ax
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  ebx,eax
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  bx,ax
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  ebx,eax
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[ecx\]
#pass
