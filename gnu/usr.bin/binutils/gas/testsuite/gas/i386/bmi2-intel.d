#as:
#objdump: -dwMintel
#name: i386 BMI2 insns (Intel disassembly)
#source: bmi2.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   ebx,eax,0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   ebx,eax,0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   ebx,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   esi,ebx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   esi,ebx,eax
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[ecx\],ebx
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   esi,DWORD PTR \[ecx\],ebx
#pass
