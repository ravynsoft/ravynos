#as:
#objdump: -dwMintel
#name: x86-64 BMI insns (Intel disassembly)
#source: x86-64-bmi.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  bx,ax
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 44 0f bc 39    	tzcnt  r15w,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 00 f2 d1       	andn   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 00 f2 11       	andn   r10d,r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 30 f7 d7       	bextr  r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 30 f7 11       	bextr  r10d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  ebx,eax
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 44 0f bc 39       	tzcnt  r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 00 f3 19       	blsi   r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 00 f3 11       	blsmsk r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 00 f3 09       	blsr   r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 f0       	andn   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 80 f2 d1       	andn   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 80 f2 11       	andn   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 f3       	bextr  rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b0 f7 d7       	bextr  r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b0 f7 11       	bextr  r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	f3 48 0f bc d8       	tzcnt  rbx,rax
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 4d 0f bc f9       	tzcnt  r15,r9
[ 	]*[a-f0-9]+:	f3 4c 0f bc 39       	tzcnt  r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d8       	blsi   rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d9       	blsi   r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 19       	blsi   r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d0       	blsmsk rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d1       	blsmsk r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 11       	blsmsk r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 c8       	blsr   rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 c9       	blsr   r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 09       	blsr   r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  bx,ax
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 44 0f bc 11    	tzcnt  r10w,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  bx,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 28 f2 f9       	andn   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 28 f2 39       	andn   r15d,r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 30 f7 fa       	bextr  r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 30 f7 39       	bextr  r15d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  ebx,eax
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 44 0f bc 11       	tzcnt  r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 28 f3 19       	blsi   r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 28 f3 11       	blsmsk r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 28 f3 09       	blsr   r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 f0       	andn   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 80 f2 d1       	andn   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 80 f2 11       	andn   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 f3       	bextr  rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b0 f7 d7       	bextr  r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b0 f7 11       	bextr  r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 0f bc d8       	tzcnt  rbx,rax
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 4d 0f bc f9       	tzcnt  r15,r9
[ 	]*[a-f0-9]+:	f3 4c 0f bc 39       	tzcnt  r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d8       	blsi   rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d9       	blsi   r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 19       	blsi   r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d0       	blsmsk rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d1       	blsmsk r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 11       	blsmsk r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 c8       	blsr   rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 c2 80 f3 c9       	blsr   r15,r9
[ 	]*[a-f0-9]+:	c4 e2 80 f3 09       	blsr   r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   rbx,QWORD PTR \[rcx\]
#pass
