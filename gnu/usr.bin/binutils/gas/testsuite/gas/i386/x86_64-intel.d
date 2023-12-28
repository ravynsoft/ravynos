#source: x86_64.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 (Intel mode)
#warning_output: x86_64.e

.*: +file format .*

Disassembly of section .text:
0+ <.*>:
[ 	]*[a-f0-9]+:	01 ca                	add    edx,ecx
[ 	]*[a-f0-9]+:	44 01 ca             	add    edx,r9d
[ 	]*[a-f0-9]+:	41 01 ca             	add    r10d,ecx
[ 	]*[a-f0-9]+:	48 01 ca             	add    rdx,rcx
[ 	]*[a-f0-9]+:	4d 01 ca             	add    r10,r9
[ 	]*[a-f0-9]+:	41 01 c0             	add    r8d,eax
[ 	]*[a-f0-9]+:	66 41 01 c0          	add    r8w,ax
[ 	]*[a-f0-9]+:	49 01 c0             	add    r8,rax
[ 	]*[a-f0-9]+:	05 11 22 33 44       	add    eax,0x44332211
[ 	]*[a-f0-9]+:	48 05 11 22 33 f4    	add    rax,0xfffffffff4332211
[ 	]*[a-f0-9]+:	66 05 33 44          	add    ax,0x4433
[ 	]*[a-f0-9]+:	48 05 11 22 33 44    	add    rax,0x44332211
[ 	]*[a-f0-9]+:	00 ca                	add    dl,cl
[ 	]*[a-f0-9]+:	00 f7                	add    bh,dh
[ 	]*[a-f0-9]+:	40 00 f7             	add    dil,sil
[ 	]*[a-f0-9]+:	41 00 f7             	add    r15b,sil
[ 	]*[a-f0-9]+:	44 00 f7             	add    dil,r14b
[ 	]*[a-f0-9]+:	45 00 f7             	add    r15b,r14b
[ 	]*[a-f0-9]+:	50                   	push   rax
[ 	]*[a-f0-9]+:	41 50                	push   r8
[ 	]*[a-f0-9]+:	41 59                	pop    r9
[ 	]*[a-f0-9]+:	04 11                	add    al,0x11
[ 	]*[a-f0-9]+:	80 c4 11             	add    ah,0x11
[ 	]*[a-f0-9]+:	40 80 c4 11          	add    spl,0x11
[ 	]*[a-f0-9]+:	41 80 c0 11          	add    r8b,0x11
[ 	]*[a-f0-9]+:	41 80 c4 11          	add    r12b,0x11
[ 	]*[a-f0-9]+:	0f 20 c0             	mov    rax,cr0
[ 	]*[a-f0-9]+:	41 0f 20 c0          	mov    r8,cr0
[ 	]*[a-f0-9]+:	44 0f 20 c0          	mov    rax,cr8
[ 	]*[a-f0-9]+:	44 0f 22 c0          	mov    cr8,rax
[ 	]*[a-f0-9]+:	f3 48 a5             	rep movs QWORD PTR es:\[rdi\],QWORD PTR ds:\[rsi\]
[ 	]*[a-f0-9]+:	66 f3 a5             	rep movs WORD PTR es:\[rdi\],WORD PTR ds:\[rsi\]
[ 	]*[a-f0-9]+:	f3 48 a5             	rep movs QWORD PTR es:\[rdi\],QWORD PTR ds:\[rsi\]
[ 	]*[a-f0-9]+:	b0 11                	mov    al,0x11
[ 	]*[a-f0-9]+:	b4 11                	mov    ah,0x11
[ 	]*[a-f0-9]+:	40 b4 11             	mov    spl,0x11
[ 	]*[a-f0-9]+:	41 b4 11             	mov    r12b,0x11
[ 	]*[a-f0-9]+:	b8 44 33 22 11       	mov    eax,0x11223344
[ 	]*[a-f0-9]+:	41 b8 44 33 22 11    	mov    r8d,0x11223344
[ 	]*[a-f0-9]+:	48 b8 88 77 66 55 44 33 22 11 	movabs rax,0x1122334455667788
[ 	]*[a-f0-9]+:	49 b8 88 77 66 55 44 33 22 11 	movabs r8,0x1122334455667788
[ 	]*[a-f0-9]+:	03 00                	add    eax,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	41 03 00             	add    eax,DWORD PTR \[r8\]
[ 	]*[a-f0-9]+:	45 03 00             	add    r8d,DWORD PTR \[r8\]
[ 	]*[a-f0-9]+:	49 03 00             	add    rax,QWORD PTR \[r8\]
[ 	]*[a-f0-9]+:	03 05 22 22 22 22    	add    eax,DWORD PTR \[rip\+0x22222222\]        # 2222[0-9a-f]* <foo\+0x2222[0-9a-f]*>
[ 	]*[a-f0-9]+:	03 45 00             	add    eax,DWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	03 04 25 22 22 22 22 	add    eax,DWORD PTR ds:0x22222222
[ 	]*[a-f0-9]+:	41 03 45 00          	add    eax,DWORD PTR \[r13\+0x0\]
[ 	]*[a-f0-9]+:	03 04 80             	add    eax,DWORD PTR \[rax\+rax\*4\]
[ 	]*[a-f0-9]+:	41 03 04 80          	add    eax,DWORD PTR \[r8\+rax\*4\]
[ 	]*[a-f0-9]+:	45 03 04 80          	add    r8d,DWORD PTR \[r8\+rax\*4\]
[ 	]*[a-f0-9]+:	43 03 04 80          	add    eax,DWORD PTR \[r8\+r8\*4\]
[ 	]*[a-f0-9]+:	46 01 04 81          	add    DWORD PTR \[rcx\+r8\*4\],r8d
[ 	]*[a-f0-9]+:	03 14 c0             	add    edx,DWORD PTR \[rax\+rax\*8\]
[ 	]*[a-f0-9]+:	03 14 c8             	add    edx,DWORD PTR \[rax\+rcx\*8\]
[ 	]*[a-f0-9]+:	03 14 d0             	add    edx,DWORD PTR \[rax\+rdx\*8\]
[ 	]*[a-f0-9]+:	03 14 d8             	add    edx,DWORD PTR \[rax\+rbx\*8\]
[ 	]*[a-f0-9]+:	03 10                	add    edx,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	03 14 e8             	add    edx,DWORD PTR \[rax\+rbp\*8\]
[ 	]*[a-f0-9]+:	03 14 f0             	add    edx,DWORD PTR \[rax\+rsi\*8\]
[ 	]*[a-f0-9]+:	03 14 f8             	add    edx,DWORD PTR \[rax\+rdi\*8\]
[ 	]*[a-f0-9]+:	42 03 14 c0          	add    edx,DWORD PTR \[rax\+r8\*8\]
[ 	]*[a-f0-9]+:	42 03 14 c8          	add    edx,DWORD PTR \[rax\+r9\*8\]
[ 	]*[a-f0-9]+:	42 03 14 d0          	add    edx,DWORD PTR \[rax\+r10\*8\]
[ 	]*[a-f0-9]+:	42 03 14 d8          	add    edx,DWORD PTR \[rax\+r11\*8\]
[ 	]*[a-f0-9]+:	42 03 14 e0          	add    edx,DWORD PTR \[rax\+r12\*8\]
[ 	]*[a-f0-9]+:	42 03 14 e8          	add    edx,DWORD PTR \[rax\+r13\*8\]
[ 	]*[a-f0-9]+:	42 03 14 f0          	add    edx,DWORD PTR \[rax\+r14\*8\]
[ 	]*[a-f0-9]+:	42 03 14 f8          	add    edx,DWORD PTR \[rax\+r15\*8\]
[ 	]*[a-f0-9]+:	83 c1 11             	add    ecx,0x11
[ 	]*[a-f0-9]+:	83 00 11             	add    DWORD PTR \[rax\],0x11
[ 	]*[a-f0-9]+:	48 83 00 11          	add    QWORD PTR \[rax\],0x11
[ 	]*[a-f0-9]+:	41 83 00 11          	add    DWORD PTR \[r8\],0x11
[ 	]*[a-f0-9]+:	83 04 81 11          	add    DWORD PTR \[rcx\+rax\*4\],0x11
[ 	]*[a-f0-9]+:	41 83 04 81 11       	add    DWORD PTR \[r9\+rax\*4\],0x11
[ 	]*[a-f0-9]+:	42 83 04 81 11       	add    DWORD PTR \[rcx\+r8\*4\],0x11
[ 	]*[a-f0-9]+:	83 05 22 22 22 22 33 	add    DWORD PTR \[rip\+0x22222222\],0x33        # 2222[0-9a-f]* <foo\+0x2222[0-9a-f]*>
[ 	]*[a-f0-9]+:	48 83 05 22 22 22 22 33 	add    QWORD PTR \[rip\+0x22222222\],0x33        # 2222[0-9a-f]* <foo\+0x2222[0-9a-f]*>
[ 	]*[a-f0-9]+:	81 05 22 22 22 22 33 33 33 33 	add    DWORD PTR \[rip\+0x22222222\],0x33333333        # 2222[0-9a-f]* <foo\+0x2222[0-9a-f]*>
[ 	]*[a-f0-9]+:	48 81 05 22 22 22 22 33 33 33 33 	add    QWORD PTR \[rip\+0x22222222\],0x33333333        # 2222[0-9a-f]* <foo\+0x2222[0-9a-f]*>
[ 	]*[a-f0-9]+:	83 04 c5 22 22 22 22 33 	add    DWORD PTR \[rax\*8\+0x22222222\],0x33
[ 	]*[a-f0-9]+:	83 80 22 22 22 22 33 	add    DWORD PTR \[rax\+0x22222222\],0x33
[ 	]*[a-f0-9]+:	83 80 22 22 22 22 33 	add    DWORD PTR \[rax\+0x22222222\],0x33
[ 	]*[a-f0-9]+:	41 83 04 e8 33       	add    DWORD PTR \[r8\+rbp\*8\],0x33
[ 	]*[a-f0-9]+:	83 04 25 22 22 22 22 33 	add    DWORD PTR ds:0x22222222,0x33
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs al,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs eax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,al
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,eax
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs rax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,rax
[ 	]*[a-f0-9]+:	48 99                	cqo
[ 	]*[a-f0-9]+:	48 98                	cdqe
[ 	]*[a-f0-9]+:	48 63 c0             	movsxd rax,eax
[ 	]*[a-f0-9]+:	48 0f bf c0          	movsx  rax,ax
[ 	]*[a-f0-9]+:	48 0f be c0          	movsx  rax,al
[ 	]*[a-f0-9]+:	cb                   	retf
[ 	]*[a-f0-9]+:	ca 10 00             	retf   0x10
[ 	]*[a-f0-9]+:	66 cb                	retfw
[ 	]*[a-f0-9]+:	66 ca 02 00          	retfw  0x2
[ 	]*[a-f0-9]+:	cb                   	retf
[ 	]*[a-f0-9]+:	ca 04 00             	retf   0x4
[ 	]*[a-f0-9]+:	48 cb                	retfq
[ 	]*[a-f0-9]+:	48 ca 08 00          	retfq  0x8

[0-9a-f]+ <bar>:
[ 	]*[a-f0-9]+:	b0 00                	mov    al,0x0
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    ax,0x0
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    eax,0x0
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    rax,0x0
[ 	]*[a-f0-9]+:	a1 00 00 00 00 00 00 00 00 	movabs eax,ds:0x0
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 00 	mov    eax,DWORD PTR ds:0x0
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    eax,DWORD PTR \[rax\+0x0\]
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    eax,DWORD PTR \[rip\+0x0\]        # [0-9a-f]+ <bar\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	b0 00                	mov    al,0x0
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    ax,0x0
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    eax,0x0
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    rax,0x0
[ 	]*[a-f0-9]+:	a1 00 00 00 00 00 00 00 00 	movabs eax,ds:0x0
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 00 	mov    eax,DWORD PTR ds:0x0
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    eax,DWORD PTR \[rax\+0x0\]
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    eax,DWORD PTR \[rip\+0x0\]        # [0-9a-f]+ <foo>

[0-9a-f]+ <foo>:
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs al,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	66 a1 11 22 33 44 55 66 77 88 	movabs ax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs eax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs rax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,al
[ 	]*[a-f0-9]+:	66 a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,ax
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,eax
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,rax
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs al,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	66 a1 11 22 33 44 55 66 77 88 	movabs ax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs eax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs rax,ds:0x8877665544332211
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,al
[ 	]*[a-f0-9]+:	66 a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,ax
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,eax
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs ds:0x8877665544332211,rax
[ 	]*[a-f0-9]+:	8a 04 25 11 22 33 ff 	mov    al,BYTE PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	66 8b 04 25 11 22 33 ff 	mov    ax,WORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	8b 04 25 11 22 33 ff 	mov    eax,DWORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	48 8b 04 25 11 22 33 ff 	mov    rax,QWORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	88 04 25 11 22 33 ff 	mov    BYTE PTR ds:0xffffffffff332211,al
[ 	]*[a-f0-9]+:	66 89 04 25 11 22 33 ff 	mov    WORD PTR ds:0xffffffffff332211,ax
[ 	]*[a-f0-9]+:	89 04 25 11 22 33 ff 	mov    DWORD PTR ds:0xffffffffff332211,eax
[ 	]*[a-f0-9]+:	48 89 04 25 11 22 33 ff 	mov    QWORD PTR ds:0xffffffffff332211,rax
[ 	]*[a-f0-9]+:	8a 04 25 11 22 33 ff 	mov    al,BYTE PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	66 8b 04 25 11 22 33 ff 	mov    ax,WORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	8b 04 25 11 22 33 ff 	mov    eax,DWORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	48 8b 04 25 11 22 33 ff 	mov    rax,QWORD PTR ds:0xffffffffff332211
[ 	]*[a-f0-9]+:	88 04 25 11 22 33 ff 	mov    BYTE PTR ds:0xffffffffff332211,al
[ 	]*[a-f0-9]+:	66 89 04 25 11 22 33 ff 	mov    WORD PTR ds:0xffffffffff332211,ax
[ 	]*[a-f0-9]+:	89 04 25 11 22 33 ff 	mov    DWORD PTR ds:0xffffffffff332211,eax
[ 	]*[a-f0-9]+:	48 89 04 25 11 22 33 ff 	mov    QWORD PTR ds:0xffffffffff332211,rax
[ 	]*[a-f0-9]+:	48 0f c7 08          	cmpxchg16b OWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f c7 08          	cmpxchg16b OWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f be f0          	movsx  si,al
[ 	]*[a-f0-9]+:	0f be f0             	movsx  esi,al
[ 	]*[a-f0-9]+:	48 0f be f0          	movsx  rsi,al
[ 	]*[a-f0-9]+:	0f bf f0             	movsx  esi,ax
[ 	]*[a-f0-9]+:	48 0f bf f0          	movsx  rsi,ax
[ 	]*[a-f0-9]+:	48 63 f0             	movsxd rsi,eax
[ 	]*[a-f0-9]+:	66 0f be 10          	movsx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f be 10             	movsx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f be 10          	movsx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f be 10          	movsx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f bf 10             	movsx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f bf 10          	movsx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzx  si,al
[ 	]*[a-f0-9]+:	0f b6 f0             	movzx  esi,al
[ 	]*[a-f0-9]+:	48 0f b6 f0          	movzx  rsi,al
[ 	]*[a-f0-9]+:	0f b7 f0             	movzx  esi,ax
[ 	]*[a-f0-9]+:	48 0f b7 f0          	movzx  rsi,ax
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b6 10             	movzx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b6 10             	movzx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b7 10             	movzx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f be f0          	movsx  si,al
[ 	]*[a-f0-9]+:	0f be f0             	movsx  esi,al
[ 	]*[a-f0-9]+:	48 0f be f0          	movsx  rsi,al
[ 	]*[a-f0-9]+:	0f bf f0             	movsx  esi,ax
[ 	]*[a-f0-9]+:	48 0f bf f0          	movsx  rsi,ax
[ 	]*[a-f0-9]+:	48 63 f0             	movsxd rsi,eax
[ 	]*[a-f0-9]+:	0f be 10             	movsx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f be 10          	movsx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f be 10          	movsx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f bf 10             	movsx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f bf 10          	movsx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzx  si,al
[ 	]*[a-f0-9]+:	0f b6 f0             	movzx  esi,al
[ 	]*[a-f0-9]+:	48 0f b6 f0          	movzx  rsi,al
[ 	]*[a-f0-9]+:	0f b7 f0             	movzx  esi,ax
[ 	]*[a-f0-9]+:	48 0f b7 f0          	movzx  rsi,ax
[ 	]*[a-f0-9]+:	0f b6 10             	movzx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b7 10             	movzx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	f3 0f 7e 0c 24       	movq   xmm1,QWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	f3 0f 7e 0c 24       	movq   xmm1,QWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	66 0f d6 0c 24       	movq   QWORD PTR \[rsp\],xmm1
[ 	]*[a-f0-9]+:	66 0f d6 0c 24       	movq   QWORD PTR \[rsp\],xmm1
[ 	]*[a-f0-9]+:	df e0                	fnstsw ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  ax
[ 	]*[a-f0-9]+:	66 0f be 00          	movsx  ax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f be 10          	movsx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f be 10             	movsx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f be 10          	movsx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f bf 10             	movsx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f bf 10          	movsx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 63 10             	movsxd rdx,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 63 00             	movsxd rax,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 00          	movzx  ax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzx  dx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b6 10             	movzx  edx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzx  rdx,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b7 10             	movzx  edx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzx  rdx,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	0f c3 00             	movnti DWORD PTR \[rax\],eax
[ 	]*[a-f0-9]+:	0f c3 00             	movnti DWORD PTR \[rax\],eax
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti QWORD PTR \[rax\],rax
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti QWORD PTR \[rax\],rax
[ 	]*[a-f0-9]+:	66 0f be 00          	movsx  ax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f be 00             	movsx  eax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f bf 00             	movsx  eax,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f bf 00          	movsx  rax,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 63 00             	movsxd rax,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 63 00             	movsxd rax,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 0f b6 00          	movzx  ax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b6 00             	movzx  eax,BYTE PTR \[rax\]
[ 	]*[a-f0-9]+:	0f b7 00             	movzx  eax,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 0f b7 00          	movzx  rax,WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	0f c3 00             	movnti DWORD PTR \[rax\],eax
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti QWORD PTR \[rax\],rax
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 00 	mov    eax,DWORD PTR (ds:)?0x0
[ 	]*[a-f0-9]+:	48 89 0c 25 00 00 00 00 	mov    QWORD PTR (ds:)?0x0,rcx
[ 	]*[a-f0-9]+:	66 0f 02 d2          	lar    dx,dx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	66 0f 02 12          	lar    dx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 02 12             	lar    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 02 12             	lar    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 12             	lldt   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	66 0f 03 d2          	lsl    dx,dx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	66 0f 03 12          	lsl    dx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 03 12             	lsl    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 03 12             	lsl    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 1a             	ltr    (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 22             	verr   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 2a             	verw   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	66 0f 02 d2          	lar    dx,dx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	0f 02 d2             	lar    edx,edx
[ 	]*[a-f0-9]+:	66 0f 02 12          	lar    dx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 02 12             	lar    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 02 12             	lar    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 d2             	lldt   dx
[ 	]*[a-f0-9]+:	0f 00 12             	lldt   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 12             	lldt   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	66 0f 03 d2          	lsl    dx,dx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	0f 03 d2             	lsl    edx,edx
[ 	]*[a-f0-9]+:	66 0f 03 12          	lsl    dx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 03 12             	lsl    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 03 12             	lsl    edx,WORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 da             	ltr    dx
[ 	]*[a-f0-9]+:	0f 00 1a             	ltr    (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 1a             	ltr    (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 e2             	verr   dx
[ 	]*[a-f0-9]+:	0f 00 22             	verr   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 22             	verr   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 ea             	verw   dx
[ 	]*[a-f0-9]+:	0f 00 2a             	verw   (WORD PTR )?\[rdx\]
[ 	]*[a-f0-9]+:	0f 00 2a             	verw   (WORD PTR )?\[rdx\]
#pass
