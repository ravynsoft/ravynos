#as:
#objdump: -dwMintel
#name: x86-64 BMI2 insns (Intel disassembly)
#source: x86-64-bmi2.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   ebx,eax,0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 43 7b f0 f9 07    	rorx   r15d,r9d,0x7
[ 	]*[a-f0-9]+:	c4 63 7b f0 39 07    	rorx   r15d,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 03 f6 d1       	mulx   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 03 f6 11       	mulx   r10d,r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 03 f5 d1       	pdep   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 03 f5 11       	pdep   r10d,r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 02 f5 d1       	pext   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 02 f5 11       	pext   r10d,r15d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 30 f5 d7       	bzhi   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 30 f5 11       	bzhi   r10d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 32 f7 d7       	sarx   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 32 f7 11       	sarx   r10d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 31 f7 d7       	shlx   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 31 f7 11       	shlx   r10d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 33 f7 d7       	shrx   r10d,r15d,r9d
[ 	]*[a-f0-9]+:	c4 62 33 f7 11       	shrx   r10d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e3 fb f0 d8 07    	rorx   rbx,rax,0x7
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   rbx,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 43 fb f0 f9 07    	rorx   r15,r9,0x7
[ 	]*[a-f0-9]+:	c4 63 fb f0 39 07    	rorx   r15,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 f0       	mulx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 83 f6 d1       	mulx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 83 f6 11       	mulx   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 f0       	pdep   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 83 f5 d1       	pdep   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 83 f5 11       	pdep   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 f0       	pext   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 82 f5 d1       	pext   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 82 f5 11       	pext   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 f3       	bzhi   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b0 f5 d7       	bzhi   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b0 f5 11       	bzhi   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 fa f7 f3       	sarx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b2 f7 d7       	sarx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b2 f7 11       	sarx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 f3       	shlx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b1 f7 d7       	shlx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b1 f7 11       	shlx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 fb f7 f3       	shrx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b3 f7 d7       	shrx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b3 f7 11       	shrx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   ebx,eax,0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 43 7b f0 d1 07    	rorx   r10d,r9d,0x7
[ 	]*[a-f0-9]+:	c4 63 7b f0 11 07    	rorx   r10d,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 2b f6 f9       	mulx   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 2b f6 39       	mulx   r15d,r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 2b f5 f9       	pdep   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 2b f5 39       	pdep   r15d,r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 2a f5 f9       	pext   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 2a f5 39       	pext   r15d,r10d,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 30 f5 fa       	bzhi   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 30 f5 39       	bzhi   r15d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 32 f7 fa       	sarx   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 32 f7 39       	sarx   r15d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 31 f7 fa       	shlx   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 31 f7 39       	shlx   r15d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 42 33 f7 fa       	shrx   r15d,r10d,r9d
[ 	]*[a-f0-9]+:	c4 62 33 f7 39       	shrx   r15d,DWORD PTR \[rcx\],r9d
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[rcx\],ebx
[ 	]*[a-f0-9]+:	c4 e3 fb f0 d8 07    	rorx   rbx,rax,0x7
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   rbx,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 43 fb f0 f9 07    	rorx   r15,r9,0x7
[ 	]*[a-f0-9]+:	c4 63 fb f0 39 07    	rorx   r15,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   rbx,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 f0       	mulx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 83 f6 d1       	mulx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 83 f6 11       	mulx   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 f0       	pdep   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 83 f5 d1       	pdep   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 83 f5 11       	pdep   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 f0       	pext   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 42 82 f5 d1       	pext   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 82 f5 11       	pext   r10,r15,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   rsi,rbx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 f3       	bzhi   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b0 f5 d7       	bzhi   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b0 f5 11       	bzhi   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 e2 fa f7 f3       	sarx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b2 f7 d7       	sarx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b2 f7 11       	sarx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 f3       	shlx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b1 f7 d7       	shlx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b1 f7 11       	shlx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 e2 fb f7 f3       	shrx   rsi,rbx,rax
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   rsi,QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	c4 42 b3 f7 d7       	shrx   r10,r15,r9
[ 	]*[a-f0-9]+:	c4 62 b3 f7 11       	shrx   r10,QWORD PTR \[rcx\],r9
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   rsi,QWORD PTR \[rcx\],rax
#pass
