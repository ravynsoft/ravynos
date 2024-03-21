#as:
#objdump: -dwMintel
#name: x86-64 TBM insns (Intel disassembly)
#source: x86-64-tbm.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f 6a 78 10 f8 00 00 00 00[ 	]+bextr  r15d,eax,0x0
[ 	]*[a-f0-9]+:	8f 4a 78 10 d7 f1 4d 00 00[ 	]+bextr  r10d,r15d,0x4df1
[ 	]*[a-f0-9]+:	8f 4a 78 10 f5 92 5e a5 2d[ 	]+bextr  r14d,r13d,0x2da55e92
[ 	]*[a-f0-9]+:	67 8f 8a 78 10 44 7d 06 ff ff ff 7f[ 	]+bextr  eax,DWORD PTR \[r13d\+r15d\*2\+0x6\],0x7fffffff
[ 	]*[a-f0-9]+:	8f ca 78 10 eb 61 f7 1e 25[ 	]+bextr  ebp,r11d,0x251ef761
[ 	]*[a-f0-9]+:	8f 6a 78 10 3c d7 39 2b 00 00[ 	]+bextr  r15d,DWORD PTR \[rdi\+rdx\*8\],0x2b39
[ 	]*[a-f0-9]+:	8f 2a 78 10 0c 35 ad de 00 00 92 00 00 00[ 	]+bextr  r9d,DWORD PTR \[r14\*1\+0xdead\],0x92
[ 	]*[a-f0-9]+:	8f ca 78 10 75 00 87 68 00 00[ 	]+bextr  esi,DWORD PTR \[r13\+0x0\],0x6887
[ 	]*[a-f0-9]+:	67 8f ca 78 10 09 0d 00 00 00[ 	]+bextr  ecx,DWORD PTR \[r9d\],0xd
[ 	]*[a-f0-9]+:	8f ea 78 10 1c 05 d8 40 00 00 2b 00 00 00[ 	]+bextr  ebx,DWORD PTR \[rax\*1\+0x40d8\],0x2b
[ 	]*[a-f0-9]+:	8f 4a 78 10 00 2d ea 00 00[ 	]+bextr  r8d,DWORD PTR \[r8\],0xea2d
[ 	]*[a-f0-9]+:	67 8f 4a 78 10 65 00 6c 00 00 00[ 	]+bextr  r12d,DWORD PTR \[r13d\+0x0\],0x6c
[ 	]*[a-f0-9]+:	8f 6a 78 10 1c 0d 8f 8c 00 00 3b 9e 00 00[ 	]+bextr  r11d,DWORD PTR \[rcx\*1\+0x8c8f\],0x9e3b
[ 	]*[a-f0-9]+:	67 8f ca 78 10 24 02 0f 00 00 00[ 	]+bextr  esp,DWORD PTR \[r10d\+eax\*1\],0xf
[ 	]*[a-f0-9]+:	67 8f aa 78 10 3c cd 00 00 00 00 ad de 00 00[ 	]+bextr  edi,DWORD PTR \[r9d\*8\+0x0\],0xdead
[ 	]*[a-f0-9]+:	8f ca 78 10 c0 fe ca 00 00[ 	]+bextr  eax,r8d,0xcafe
[ 	]*[a-f0-9]+:	8f 4a f8 10 81 bc 10 00 00 b9 3b 26 7d[ 	]+bextr  r8,QWORD PTR \[r9\+0x10bc\],0x7d263bb9
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 3c 65 00 00 00 00 67 00 00 00[ 	]+bextr  r15,QWORD PTR \[r12d\*2\+0x0\],0x67
[ 	]*[a-f0-9]+:	8f ea f8 10 c0 00 00 00 00[ 	]+bextr  rax,rax,0x0
[ 	]*[a-f0-9]+:	67 8f ea f8 10 26 9b 53 00 00[ 	]+bextr  rsp,QWORD PTR \[esi\],0x539b
[ 	]*[a-f0-9]+:	8f ca f8 10 08 ff ff ff 7f[ 	]+bextr  rcx,QWORD PTR \[r8\],0x7fffffff
[ 	]*[a-f0-9]+:	67 8f ea f8 10 04 3d ff ff ff 3f 01 00 00 00[ 	]+bextr  rax,QWORD PTR \[edi\*1\+0x3fffffff\],0x1
[ 	]*[a-f0-9]+:	67 8f 8a f8 10 b4 30 84 dd ff ff 9e 00 00 00[ 	]+bextr  rsi,QWORD PTR \[r8d\+r14d\*1\-0x227c\],0x9e
[ 	]*[a-f0-9]+:	8f ca f8 10 c7 64 c4 a6 02[ 	]+bextr  rax,r15,0x2a6c464
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 4c 1f 02 04 00 00 00[ 	]+bextr  r9,QWORD PTR \[edi\+r11d\*1\+0x2\],0x4
[ 	]*[a-f0-9]+:	8f ea f8 10 ef 02 00 00 00[ 	]+bextr  rbp,rdi,0x2
[ 	]*[a-f0-9]+:	67 8f ca f8 10 14 16 fb 7e 1e 78[ 	]+bextr  rdx,QWORD PTR \[r14d\+edx\*1\],0x781e7efb
[ 	]*[a-f0-9]+:	8f 0a f8 10 ac 2b 68 db 00 00 39 40 cb 70[ 	]+bextr  r13,QWORD PTR \[r11\+r13\*1\+0xdb68\],0x70cb4039
[ 	]*[a-f0-9]+:	8f 4a f8 10 16 73 13 00 00[ 	]+bextr  r10,QWORD PTR \[r14\],0x1373
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 3c af 6d 55 00 00[ 	]+bextr  r15,QWORD PTR \[edi\+r13d\*4\],0x556d
[ 	]*[a-f0-9]+:	8f 4a f8 10 11 00 00 00 00[ 	]+bextr  r10,QWORD PTR \[r9\],0x0
[ 	]*[a-f0-9]+:	8f 6a f8 10 1f ef ee ee 7b[ 	]+bextr  r11,QWORD PTR \[rdi\],0x7beeeeef
[ 	]*[a-f0-9]+:	8f e9 00 01 cc[ 	]+blcfill r15d,esp
[ 	]*[a-f0-9]+:	8f a9 68 01 0c a6[ 	]+blcfill edx,DWORD PTR \[rsi\+r12\*4\]
[ 	]*[a-f0-9]+:	67 8f e9 08 01 08[ 	]+blcfill r14d,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f a9 50 01 0c ad 00 00 00 00[ 	]+blcfill ebp,DWORD PTR \[r13\*4\+0x0\]
[ 	]*[a-f0-9]+:	67 8f c9 78 01 0e[ 	]+blcfill eax,DWORD PTR \[r14d\]
[ 	]*[a-f0-9]+:	8f c9 30 01 0b[ 	]+blcfill r9d,DWORD PTR \[r11\]
[ 	]*[a-f0-9]+:	8f a9 10 01 0c 45 ad de 00 00[ 	]+blcfill r13d,DWORD PTR \[r8\*2\+0xdead\]
[ 	]*[a-f0-9]+:	8f c9 00 01 cf[ 	]+blcfill r15d,r15d
[ 	]*[a-f0-9]+:	8f c9 40 01 ce[ 	]+blcfill edi,r14d
[ 	]*[a-f0-9]+:	8f e9 20 01 c8[ 	]+blcfill r11d,eax
[ 	]*[a-f0-9]+:	8f c9 18 01 c9[ 	]+blcfill r12d,r9d
[ 	]*[a-f0-9]+:	67 8f c9 60 01 4d 67[ 	]+blcfill ebx,DWORD PTR \[r13d\+0x67\]
[ 	]*[a-f0-9]+:	67 8f e9 00 01 0b[ 	]+blcfill r15d,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	67 8f a9 08 01 4c 19 0b[ 	]+blcfill r14d,DWORD PTR \[ecx\+r11d\*1\+0xb\]
[ 	]*[a-f0-9]+:	8f c9 78 01 8d 4a ff ff ff[ 	]+blcfill eax,DWORD PTR \[r13\-0xb6\]
[ 	]*[a-f0-9]+:	8f c9 48 01 09[ 	]+blcfill esi,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f c9 f8 01 cf[ 	]+blcfill rax,r15
[ 	]*[a-f0-9]+:	8f c9 a0 01 cd[ 	]+blcfill r11,r13
[ 	]*[a-f0-9]+:	8f c9 e0 01 c8[ 	]+blcfill rbx,r8
[ 	]*[a-f0-9]+:	67 8f c9 80 01 0f[ 	]+blcfill r15,QWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	67 8f c9 88 01 4d 00[ 	]+blcfill r14,QWORD PTR \[r13d\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 b0 01 c8[ 	]+blcfill r9,rax
[ 	]*[a-f0-9]+:	8f 89 e8 01 4c 24 0a[ 	]+blcfill rdx,QWORD PTR \[r12\+r12\*1\+0xa\]
[ 	]*[a-f0-9]+:	8f c9 98 01 ce[ 	]+blcfill r12,r14
[ 	]*[a-f0-9]+:	8f e9 a8 01 cf[ 	]+blcfill r10,rdi
[ 	]*[a-f0-9]+:	67 8f c9 90 01 0b[ 	]+blcfill r13,QWORD PTR \[r11d\]
[ 	]*[a-f0-9]+:	67 8f e9 b8 01 0c 15 25 c6 ff ff[ 	]+blcfill r8,QWORD PTR \[edx\*1\-0x39db\]
[ 	]*[a-f0-9]+:	8f c9 d8 01 0c 34[ 	]+blcfill rsp,QWORD PTR \[r12\+rsi\*1\]
[ 	]*[a-f0-9]+:	67 8f 89 b8 01 4c 6d 00[ 	]+blcfill r8,QWORD PTR \[r13d\+r13d\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 d0 01 08[ 	]+blcfill rbp,QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	8f c9 80 01 09[ 	]+blcfill r15,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f c9 f0 01 cb[ 	]+blcfill rcx,r11
[ 	]*[a-f0-9]+:	8f c9 78 02 f7[ 	]+blci   eax,r15d
[ 	]*[a-f0-9]+:	8f e9 00 02 32[ 	]+blci   r15d,DWORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	8f e9 28 02 f0[ 	]+blci   r10d,eax
[ 	]*[a-f0-9]+:	67 8f e9 38 02 37[ 	]+blci   r8d,DWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	67 8f c9 68 02 75 00[ 	]+blci   edx,DWORD PTR \[r13d\+0x0\]
[ 	]*[a-f0-9]+:	67 8f e9 20 02 32[ 	]+blci   r11d,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	67 8f e9 18 02 34 05 37 09 00 00[ 	]+blci   r12d,DWORD PTR \[eax\*1\+0x937\]
[ 	]*[a-f0-9]+:	8f c9 70 02 31[ 	]+blci   ecx,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f c9 58 02 31[ 	]+blci   esp,DWORD PTR \[r9d\]
[ 	]*[a-f0-9]+:	8f e9 48 02 f2[ 	]+blci   esi,edx
[ 	]*[a-f0-9]+:	8f e9 08 02 f5[ 	]+blci   r14d,ebp
[ 	]*[a-f0-9]+:	8f e9 78 02 f3[ 	]+blci   eax,ebx
[ 	]*[a-f0-9]+:	8f e9 38 02 30[ 	]+blci   r8d,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	67 8f a9 40 02 34 75 00 00 00 00[ 	]+blci   edi,DWORD PTR \[r14d\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 78 02 33[ 	]+blci   eax,DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	67 8f 89 30 02 b4 31 31 a3 4c 43[ 	]+blci   r9d,DWORD PTR \[r9d\+r14d\*1\+0x434ca331\]
[ 	]*[a-f0-9]+:	67 8f e9 a0 02 33[ 	]+blci   r11,QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f c9 f8 02 37[ 	]+blci   rax,QWORD PTR \[r15\]
[ 	]*[a-f0-9]+:	67 8f c9 80 02 34 dc[ 	]+blci   r15,QWORD PTR \[r12d\+ebx\*8\]
[ 	]*[a-f0-9]+:	8f c9 d0 02 f7[ 	]+blci   rbp,r15
[ 	]*[a-f0-9]+:	67 8f e9 d8 02 34 33[ 	]+blci   rsp,QWORD PTR \[ebx\+esi\*1\]
[ 	]*[a-f0-9]+:	8f c9 f0 02 f4[ 	]+blci   rcx,r12
[ 	]*[a-f0-9]+:	8f c9 c0 02 31[ 	]+blci   rdi,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f c9 e0 02 34 3c[ 	]+blci   rbx,QWORD PTR \[r12d\+edi\*1\]
[ 	]*[a-f0-9]+:	8f e9 80 02 34 d5 19 5b 00 00[ 	]+blci   r15,QWORD PTR \[rdx\*8\+0x5b19\]
[ 	]*[a-f0-9]+:	67 8f e9 a8 02 34 c5 00 00 00 00[ 	]+blci   r10,QWORD PTR \[eax\*8\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 b8 02 33[ 	]+blci   r8,QWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	67 8f e9 b0 02 b4 50 0b ff ff ff[ 	]+blci   r9,QWORD PTR \[eax\+edx\*2\-0xf5\]
[ 	]*[a-f0-9]+:	8f c9 88 02 75 00[ 	]+blci   r14,QWORD PTR \[r13\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 f8 02 f5[ 	]+blci   rax,rbp
[ 	]*[a-f0-9]+:	67 8f e9 90 02 30[ 	]+blci   r13,QWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f c9 e8 02 34 24[ 	]+blci   rdx,QWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	67 8f c9 00 01 2c c6[ 	]+blcic  r15d,DWORD PTR \[r14d\+eax\*8\]
[ 	]*[a-f0-9]+:	8f c9 78 01 ef[ 	]+blcic  eax,r15d
[ 	]*[a-f0-9]+:	8f c9 38 01 29[ 	]+blcic  r8d,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f c9 30 01 2c 59[ 	]+blcic  r9d,DWORD PTR \[r9\+rbx\*2\]
[ 	]*[a-f0-9]+:	67 8f e9 48 01 2b[ 	]+blcic  esi,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	67 8f e9 50 01 2c 05 fe ff ff ff[ 	]+blcic  ebp,DWORD PTR \[eax\*1\-0x2\]
[ 	]*[a-f0-9]+:	8f e9 60 01 28[ 	]+blcic  ebx,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	8f c9 40 01 2b[ 	]+blcic  edi,DWORD PTR \[r11\]
[ 	]*[a-f0-9]+:	8f e9 20 01 e8[ 	]+blcic  r11d,eax
[ 	]*[a-f0-9]+:	8f c9 18 01 2e[ 	]+blcic  r12d,DWORD PTR \[r14\]
[ 	]*[a-f0-9]+:	8f c9 78 01 eb[ 	]+blcic  eax,r11d
[ 	]*[a-f0-9]+:	8f a9 00 01 2c 1d a7 d0 1a 14[ 	]+blcic  r15d,DWORD PTR \[r11\*1\+0x141ad0a7\]
[ 	]*[a-f0-9]+:	8f a9 10 01 2c 88[ 	]+blcic  r13d,DWORD PTR \[rax\+r9\*4\]
[ 	]*[a-f0-9]+:	8f e9 00 01 2b[ 	]+blcic  r15d,DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	67 8f 89 28 01 2c 3f[ 	]+blcic  r10d,DWORD PTR \[r15d\+r15d\*1\]
[ 	]*[a-f0-9]+:	67 8f c9 68 01 29[ 	]+blcic  edx,DWORD PTR \[r9d\]
[ 	]*[a-f0-9]+:	67 8f a9 f0 01 2c 2d b3 cb d3 59[ 	]+blcic  rcx,QWORD PTR \[r13d\*1\+0x59d3cbb3\]
[ 	]*[a-f0-9]+:	8f c9 f8 01 ee[ 	]+blcic  rax,r14
[ 	]*[a-f0-9]+:	67 8f c9 80 01 2c 24[ 	]+blcic  r15,QWORD PTR \[r12d\]
[ 	]*[a-f0-9]+:	8f e9 88 01 e8[ 	]+blcic  r14,rax
[ 	]*[a-f0-9]+:	8f c9 d0 01 ef[ 	]+blcic  rbp,r15
[ 	]*[a-f0-9]+:	8f e9 d8 01 2b[ 	]+blcic  rsp,QWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	8f e9 e8 01 eb[ 	]+blcic  rdx,rbx
[ 	]*[a-f0-9]+:	8f c9 c0 01 e8[ 	]+blcic  rdi,r8
[ 	]*[a-f0-9]+:	8f c9 c8 01 29[ 	]+blcic  rsi,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f e9 c0 01 2c c5 db db 00 00[ 	]+blcic  rdi,QWORD PTR \[rax\*8\+0xdbdb\]
[ 	]*[a-f0-9]+:	8f c9 e0 01 ea[ 	]+blcic  rbx,r10
[ 	]*[a-f0-9]+:	67 8f e9 a0 01 2b[ 	]+blcic  r11,QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f c9 b0 01 ed[ 	]+blcic  r9,r13
[ 	]*[a-f0-9]+:	8f c9 f8 01 28[ 	]+blcic  rax,QWORD PTR \[r8\]
[ 	]*[a-f0-9]+:	8f 89 98 01 ac 12 ad de 00 00[ 	]+blcic  r12,QWORD PTR \[r10\+r10\*1\+0xdead\]
[ 	]*[a-f0-9]+:	67 8f e9 f0 01 2c 02[ 	]+blcic  rcx,QWORD PTR \[edx\+eax\*1\]
[ 	]*[a-f0-9]+:	67 8f e9 00 02 09[ 	]+blcmsk r15d,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f e9 78 02 cd[ 	]+blcmsk eax,ebp
[ 	]*[a-f0-9]+:	67 8f e9 40 02 0b[ 	]+blcmsk edi,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 68 02 c8[ 	]+blcmsk edx,eax
[ 	]*[a-f0-9]+:	8f a9 10 02 0c d5 00 00 00 00[ 	]+blcmsk r13d,DWORD PTR \[r10\*8\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 30 02 09[ 	]+blcmsk r9d,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f c9 18 02 0a[ 	]+blcmsk r12d,DWORD PTR \[r10\]
[ 	]*[a-f0-9]+:	8f e9 60 02 c9[ 	]+blcmsk ebx,ecx
[ 	]*[a-f0-9]+:	67 8f e9 78 02 0a[ 	]+blcmsk eax,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	8f e9 20 02 ce[ 	]+blcmsk r11d,esi
[ 	]*[a-f0-9]+:	8f a9 00 02 0c b5 00 00 00 00[ 	]+blcmsk r15d,DWORD PTR \[r14\*4\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 78 02 cf[ 	]+blcmsk eax,r15d
[ 	]*[a-f0-9]+:	67 8f c9 08 02 8e 5f f3 00 00[ 	]+blcmsk r14d,DWORD PTR \[r14d\+0xf35f\]
[ 	]*[a-f0-9]+:	67 8f c9 38 02 0c 30[ 	]+blcmsk r8d,DWORD PTR \[r8d\+esi\*1\]
[ 	]*[a-f0-9]+:	8f c9 58 02 0c 14[ 	]+blcmsk esp,DWORD PTR \[r12\+rdx\*1\]
[ 	]*[a-f0-9]+:	67 8f c9 28 02 08[ 	]+blcmsk r10d,DWORD PTR \[r8d\]
[ 	]*[a-f0-9]+:	67 8f a9 98 02 0c 2d 00 00 00 00[ 	]+blcmsk r12,QWORD PTR \[r13d\*1\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 e0 02 cf[ 	]+blcmsk rbx,r15
[ 	]*[a-f0-9]+:	8f e9 80 02 c8[ 	]+blcmsk r15,rax
[ 	]*[a-f0-9]+:	67 8f a9 b8 02 0c 0d 03 00 00 00[ 	]+blcmsk r8,QWORD PTR \[r9d\*1\+0x3\]
[ 	]*[a-f0-9]+:	8f 89 d0 02 8c 79 02 35 ff ff[ 	]+blcmsk rbp,QWORD PTR \[r9\+r15\*2\-0xcafe\]
[ 	]*[a-f0-9]+:	8f c9 d8 02 4d 00[ 	]+blcmsk rsp,QWORD PTR \[r13\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 f8 02 0a[ 	]+blcmsk rax,QWORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	8f c9 90 02 0c 24[ 	]+blcmsk r13,QWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	8f e9 e8 02 0c d5 f9 ff ff ff[ 	]+blcmsk rdx,QWORD PTR \[rdx\*8\-0x7\]
[ 	]*[a-f0-9]+:	8f c9 88 02 0b[ 	]+blcmsk r14,QWORD PTR \[r11\]
[ 	]*[a-f0-9]+:	8f c9 b0 02 ce[ 	]+blcmsk r9,r14
[ 	]*[a-f0-9]+:	8f e9 a0 02 09[ 	]+blcmsk r11,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	67 8f c9 f8 02 0e[ 	]+blcmsk rax,QWORD PTR \[r14d\]
[ 	]*[a-f0-9]+:	8f e9 c0 02 0c c5 00 00 00 00[ 	]+blcmsk rdi,QWORD PTR \[rax\*8\+0x0\]
[ 	]*[a-f0-9]+:	67 8f c9 90 02 0f[ 	]+blcmsk r13,QWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	67 8f e9 88 02 0c 33[ 	]+blcmsk r14,QWORD PTR \[ebx\+esi\*1\]
[ 	]*[a-f0-9]+:	8f e9 00 01 18[ 	]+blcs   r15d,DWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	67 8f a9 38 01 1c 05 01 00 00 00[ 	]+blcs   r8d,DWORD PTR \[r8d\*1\+0x1\]
[ 	]*[a-f0-9]+:	8f c9 70 01 da[ 	]+blcs   ecx,r10d
[ 	]*[a-f0-9]+:	8f c9 28 01 df[ 	]+blcs   r10d,r15d
[ 	]*[a-f0-9]+:	8f c9 78 01 db[ 	]+blcs   eax,r11d
[ 	]*[a-f0-9]+:	67 8f e9 40 01 99 9b dc 68 81[ 	]+blcs   edi,DWORD PTR \[ecx\-0x7e972365\]
[ 	]*[a-f0-9]+:	67 8f e9 08 01 1e[ 	]+blcs   r14d,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f c9 20 01 5a fd[ 	]+blcs   r11d,DWORD PTR \[r10\-0x3\]
[ 	]*[a-f0-9]+:	8f e9 58 01 1f[ 	]+blcs   esp,DWORD PTR \[rdi\]
[ 	]*[a-f0-9]+:	67 8f c9 60 01 1f[ 	]+blcs   ebx,DWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	8f c9 10 01 1c b1[ 	]+blcs   r13d,DWORD PTR \[r9\+rsi\*4\]
[ 	]*[a-f0-9]+:	8f c9 30 01 1c 19[ 	]+blcs   r9d,DWORD PTR \[r9\+rbx\*1\]
[ 	]*[a-f0-9]+:	67 8f e9 00 01 1c 08[ 	]+blcs   r15d,DWORD PTR \[eax\+ecx\*1\]
[ 	]*[a-f0-9]+:	8f e9 48 01 db[ 	]+blcs   esi,ebx
[ 	]*[a-f0-9]+:	8f e9 78 01 de[ 	]+blcs   eax,esi
[ 	]*[a-f0-9]+:	8f e9 18 01 df[ 	]+blcs   r12d,edi
[ 	]*[a-f0-9]+:	8f e9 f8 01 df[ 	]+blcs   rax,rdi
[ 	]*[a-f0-9]+:	8f e9 98 01 18[ 	]+blcs   r12,QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	8f c9 80 01 df[ 	]+blcs   r15,r15
[ 	]*[a-f0-9]+:	8f c9 f0 01 da[ 	]+blcs   rcx,r10
[ 	]*[a-f0-9]+:	67 8f e9 90 01 18[ 	]+blcs   r13,QWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 b8 01 d8[ 	]+blcs   r8,rax
[ 	]*[a-f0-9]+:	67 8f e9 c0 01 5a ff[ 	]+blcs   rdi,QWORD PTR \[edx\-0x1\]
[ 	]*[a-f0-9]+:	8f e9 a0 01 db[ 	]+blcs   r11,rbx
[ 	]*[a-f0-9]+:	67 8f e9 d8 01 1c 45 00 00 00 00[ 	]+blcs   rsp,QWORD PTR \[eax\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f 89 a8 01 1c 29[ 	]+blcs   r10,QWORD PTR \[r9\+r13\*1\]
[ 	]*[a-f0-9]+:	67 8f a9 88 01 1c 05 cf 1d 00 00[ 	]+blcs   r14,QWORD PTR \[r8d\*1\+0x1dcf\]
[ 	]*[a-f0-9]+:	67 8f a9 80 01 1c bd 00 00 00 00[ 	]+blcs   r15,QWORD PTR \[r15d\*4\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 d0 01 19[ 	]+blcs   rbp,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f c9 e8 01 5c 05 00[ 	]+blcs   rdx,QWORD PTR \[r13d\+eax\*1\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 d8 01 dc[ 	]+blcs   rsp,r12
[ 	]*[a-f0-9]+:	8f e9 e0 01 1f[ 	]+blcs   rbx,QWORD PTR \[rdi\]
[ 	]*[a-f0-9]+:	67 8f e9 68 01 16[ 	]+blsfill edx,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f c9 78 01 11[ 	]+blsfill eax,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f e9 00 01 13[ 	]+blsfill r15d,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 20 01 d0[ 	]+blsfill r11d,eax
[ 	]*[a-f0-9]+:	8f c9 38 01 14 24[ 	]+blsfill r8d,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	67 8f a9 00 01 14 0d 7e aa ff ff[ 	]+blsfill r15d,DWORD PTR \[r9d\*1\-0x5582\]
[ 	]*[a-f0-9]+:	8f e9 78 01 d4[ 	]+blsfill eax,esp
[ 	]*[a-f0-9]+:	67 8f a9 50 01 14 65 00 00 00 00[ 	]+blsfill ebp,DWORD PTR \[r12d\*2\+0x0\]
[ 	]*[a-f0-9]+:	67 8f c9 60 01 10[ 	]+blsfill ebx,DWORD PTR \[r8d\]
[ 	]*[a-f0-9]+:	67 8f e9 58 01 10[ 	]+blsfill esp,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f a9 18 01 14 1d 03 4f 00 00[ 	]+blsfill r12d,DWORD PTR \[r11\*1\+0x4f03\]
[ 	]*[a-f0-9]+:	67 8f a9 78 01 14 15 0f 00 00 00[ 	]+blsfill eax,DWORD PTR \[r10d\*1\+0xf\]
[ 	]*[a-f0-9]+:	67 8f c9 40 01 17[ 	]+blsfill edi,DWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	8f e9 70 01 14 35 8f 22 00 00[ 	]+blsfill ecx,DWORD PTR \[rsi\*1\+0x228f\]
[ 	]*[a-f0-9]+:	67 8f e9 48 01 11[ 	]+blsfill esi,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	8f c9 10 01 d0[ 	]+blsfill r13d,r8d
[ 	]*[a-f0-9]+:	67 8f e9 80 01 14 85 f4 ff ff ff[ 	]+blsfill r15,QWORD PTR \[eax\*4\-0xc\]
[ 	]*[a-f0-9]+:	8f e9 98 01 d0[ 	]+blsfill r12,rax
[ 	]*[a-f0-9]+:	8f e9 f8 01 d2[ 	]+blsfill rax,rdx
[ 	]*[a-f0-9]+:	8f c9 d0 01 11[ 	]+blsfill rbp,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f e9 e0 01 17[ 	]+blsfill rbx,QWORD PTR \[edi\]
[ 	]*[a-f0-9]+:	8f c9 b0 01 d7[ 	]+blsfill r9,r15
[ 	]*[a-f0-9]+:	8f e9 d8 01 d3[ 	]+blsfill rsp,rbx
[ 	]*[a-f0-9]+:	8f c9 f8 01 17[ 	]+blsfill rax,QWORD PTR \[r15\]
[ 	]*[a-f0-9]+:	67 8f e9 a8 01 94 3f b9 56 00 00[ 	]+blsfill r10,QWORD PTR \[edi\+edi\*1\+0x56b9\]
[ 	]*[a-f0-9]+:	67 8f c9 f0 01 94 b4 2f d4 ff ff[ 	]+blsfill rcx,QWORD PTR \[r12d\+esi\*4\-0x2bd1\]
[ 	]*[a-f0-9]+:	8f c9 d8 01 13[ 	]+blsfill rsp,QWORD PTR \[r11\]
[ 	]*[a-f0-9]+:	8f c9 b8 01 d5[ 	]+blsfill r8,r13
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 14 43[ 	]+blsfill rax,QWORD PTR \[ebx\+eax\*2\]
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 13[ 	]+blsfill rax,QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 a0 01 14 13[ 	]+blsfill r11,QWORD PTR \[rbx\+rdx\*1\]
[ 	]*[a-f0-9]+:	8f c9 c8 01 95 dc 2f 00 00[ 	]+blsfill rsi,QWORD PTR \[r13\+0x2fdc\]
[ 	]*[a-f0-9]+:	8f c9 00 01 f3[ 	]+blsic  r15d,r11d
[ 	]*[a-f0-9]+:	8f e9 50 01 34 35 61 86 ff ff[ 	]+blsic  ebp,DWORD PTR \[rsi\*1\-0x799f\]
[ 	]*[a-f0-9]+:	8f c9 78 01 f7[ 	]+blsic  eax,r15d
[ 	]*[a-f0-9]+:	8f a9 70 01 34 10[ 	]+blsic  ecx,DWORD PTR \[rax\+r10\*1\]
[ 	]*[a-f0-9]+:	8f e9 28 01 f0[ 	]+blsic  r10d,eax
[ 	]*[a-f0-9]+:	67 8f c9 30 01 75 00[ 	]+blsic  r9d,DWORD PTR \[r13d\+0x0\]
[ 	]*[a-f0-9]+:	8f c9 60 01 31[ 	]+blsic  ebx,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f e9 58 01 33[ 	]+blsic  esp,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	67 8f c9 20 01 34 24[ 	]+blsic  r11d,DWORD PTR \[r12d\]
[ 	]*[a-f0-9]+:	8f e9 68 01 34 3d fe bc 00 00[ 	]+blsic  edx,DWORD PTR \[rdi\*1\+0xbcfe\]
[ 	]*[a-f0-9]+:	67 8f c9 40 01 36[ 	]+blsic  edi,DWORD PTR \[r14d\]
[ 	]*[a-f0-9]+:	67 8f a9 00 01 34 2d ec 78 00 00[ 	]+blsic  r15d,DWORD PTR \[r13d\*1\+0x78ec\]
[ 	]*[a-f0-9]+:	67 8f c9 48 01 33[ 	]+blsic  esi,DWORD PTR \[r11d\]
[ 	]*[a-f0-9]+:	8f c9 08 01 32[ 	]+blsic  r14d,DWORD PTR \[r10\]
[ 	]*[a-f0-9]+:	67 8f c9 00 01 31[ 	]+blsic  r15d,DWORD PTR \[r9d\]
[ 	]*[a-f0-9]+:	8f c9 00 01 f2[ 	]+blsic  r15d,r10d
[ 	]*[a-f0-9]+:	8f c9 f8 01 f7[ 	]+blsic  rax,r15
[ 	]*[a-f0-9]+:	8f e9 b0 01 34 05 67 00 00 00[ 	]+blsic  r9,QWORD PTR \[rax\*1\+0x67\]
[ 	]*[a-f0-9]+:	67 8f 89 e8 01 34 20[ 	]+blsic  rdx,QWORD PTR \[r8d\+r12d\*1\]
[ 	]*[a-f0-9]+:	67 8f c9 80 01 37[ 	]+blsic  r15,QWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	8f c9 f0 01 f1[ 	]+blsic  rcx,r9
[ 	]*[a-f0-9]+:	8f c9 c0 01 f2[ 	]+blsic  rdi,r10
[ 	]*[a-f0-9]+:	8f a9 e0 01 34 05 ff ff ff 3f[ 	]+blsic  rbx,QWORD PTR \[r8\*1\+0x3fffffff\]
[ 	]*[a-f0-9]+:	8f e9 80 01 f2[ 	]+blsic  r15,rdx
[ 	]*[a-f0-9]+:	8f e9 c8 01 30[ 	]+blsic  rsi,QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	67 8f c9 f8 01 37[ 	]+blsic  rax,QWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	8f e9 80 01 33[ 	]+blsic  r15,QWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	8f e9 b8 01 f0[ 	]+blsic  r8,rax
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 33[ 	]+blsic  rax,QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 88 01 f1[ 	]+blsic  r14,rcx
[ 	]*[a-f0-9]+:	67 8f c9 c8 01 34 07[ 	]+blsic  rsi,QWORD PTR \[r15d\+eax\*1\]
[ 	]*[a-f0-9]+:	8f c9 98 01 f5[ 	]+blsic  r12,r13
[ 	]*[a-f0-9]+:	8f e9 00 01 7e fd[ 	]+t1mskc r15d,DWORD PTR \[rsi\-0x3\]
[ 	]*[a-f0-9]+:	8f c9 18 01 ff[ 	]+t1mskc r12d,r15d
[ 	]*[a-f0-9]+:	8f c9 30 01 3c 24[ 	]+t1mskc r9d,DWORD PTR \[r12\]
[ 	]*[a-f0-9]+:	8f e9 78 01 fe[ 	]+t1mskc eax,esi
[ 	]*[a-f0-9]+:	67 8f c9 58 01 7a fe[ 	]+t1mskc esp,DWORD PTR \[r10d\-0x2\]
[ 	]*[a-f0-9]+:	67 8f e9 10 01 3c 45 00 00 00 00[ 	]+t1mskc r13d,DWORD PTR \[eax\*2\+0x0\]
[ 	]*[a-f0-9]+:	8f e9 48 01 f8[ 	]+t1mskc esi,eax
[ 	]*[a-f0-9]+:	67 8f c9 78 01 3c 24[ 	]+t1mskc eax,DWORD PTR \[r12d\]
[ 	]*[a-f0-9]+:	8f e9 28 01 3c 1d 9c f5 00 00[ 	]+t1mskc r10d,DWORD PTR \[rbx\*1\+0xf59c\]
[ 	]*[a-f0-9]+:	67 8f e9 20 01 3c 85 00 00 00 00[ 	]+t1mskc r11d,DWORD PTR \[eax\*4\+0x0\]
[ 	]*[a-f0-9]+:	67 8f e9 38 01 3b[ 	]+t1mskc r8d,DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 60 01 ff[ 	]+t1mskc ebx,edi
[ 	]*[a-f0-9]+:	67 8f e9 08 01 3a[ 	]+t1mskc r14d,DWORD PTR \[edx\]
[ 	]*[a-f0-9]+:	67 8f c9 00 01 3b[ 	]+t1mskc r15d,DWORD PTR \[r11d\]
[ 	]*[a-f0-9]+:	67 8f e9 70 01 3e[ 	]+t1mskc ecx,DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	8f 89 40 01 3c 29[ 	]+t1mskc edi,DWORD PTR \[r9\+r13\*1\]
[ 	]*[a-f0-9]+:	8f c9 d8 01 be ff ff ff 3f[ 	]+t1mskc rsp,QWORD PTR \[r14\+0x3fffffff\]
[ 	]*[a-f0-9]+:	8f e9 f8 01 f8[ 	]+t1mskc rax,rax
[ 	]*[a-f0-9]+:	8f c9 e0 01 38[ 	]+t1mskc rbx,QWORD PTR \[r8\]
[ 	]*[a-f0-9]+:	67 8f c9 c0 01 3c 3c[ 	]+t1mskc rdi,QWORD PTR \[r12d\+edi\*1\]
[ 	]*[a-f0-9]+:	8f c9 f0 01 fb[ 	]+t1mskc rcx,r11
[ 	]*[a-f0-9]+:	8f c9 88 01 7d 00[ 	]+t1mskc r14,QWORD PTR \[r13\+0x0\]
[ 	]*[a-f0-9]+:	67 8f e9 e8 01 3c c5 ad de 00 00[ 	]+t1mskc rdx,QWORD PTR \[eax\*8\+0xdead\]
[ 	]*[a-f0-9]+:	8f c9 80 01 ff[ 	]+t1mskc r15,r15
[ 	]*[a-f0-9]+:	8f c9 d0 01 3f[ 	]+t1mskc rbp,QWORD PTR \[r15\]
[ 	]*[a-f0-9]+:	8f e9 b0 01 fc[ 	]+t1mskc r9,rsp
[ 	]*[a-f0-9]+:	8f e9 c8 01 3a[ 	]+t1mskc rsi,QWORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	8f c9 a8 01 fa[ 	]+t1mskc r10,r10
[ 	]*[a-f0-9]+:	67 8f c9 90 01 39[ 	]+t1mskc r13,QWORD PTR \[r9d\]
[ 	]*[a-f0-9]+:	8f e9 f8 01 fb[ 	]+t1mskc rax,rbx
[ 	]*[a-f0-9]+:	8f c9 f8 01 39[ 	]+t1mskc rax,QWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	67 8f c9 a8 01 38[ 	]+t1mskc r10,QWORD PTR \[r8d\]
[ 	]*[a-f0-9]+:	8f e9 28 01 e3[ 	]+tzmsk  r10d,ebx
[ 	]*[a-f0-9]+:	8f c9 78 01 21[ 	]+tzmsk  eax,DWORD PTR \[r9\]
[ 	]*[a-f0-9]+:	8f e9 00 01 22[ 	]+tzmsk  r15d,DWORD PTR \[rdx\]
[ 	]*[a-f0-9]+:	8f e9 18 01 e5[ 	]+tzmsk  r12d,ebp
[ 	]*[a-f0-9]+:	8f c9 10 01 e2[ 	]+tzmsk  r13d,r10d
[ 	]*[a-f0-9]+:	8f c9 00 01 e7[ 	]+tzmsk  r15d,r15d
[ 	]*[a-f0-9]+:	8f 89 60 01 a4 0b 02 35 ff ff[ 	]+tzmsk  ebx,DWORD PTR \[r11\+r9\*1\-0xcafe\]
[ 	]*[a-f0-9]+:	67 8f a9 68 01 64 2e 01[ 	]+tzmsk  edx,DWORD PTR \[esi\+r13d\*1\+0x1\]
[ 	]*[a-f0-9]+:	67 8f c9 08 01 23[ 	]+tzmsk  r14d,DWORD PTR \[r11d\]
[ 	]*[a-f0-9]+:	67 8f a9 70 01 24 a1[ 	]+tzmsk  ecx,DWORD PTR \[ecx\+r12d\*4\]
[ 	]*[a-f0-9]+:	67 8f e9 30 01 20[ 	]+tzmsk  r9d,DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8f e9 38 01 60 fa[ 	]+tzmsk  r8d,DWORD PTR \[rax\-0x6\]
[ 	]*[a-f0-9]+:	8f e9 48 01 e7[ 	]+tzmsk  esi,edi
[ 	]*[a-f0-9]+:	8f e9 00 01 e0[ 	]+tzmsk  r15d,eax
[ 	]*[a-f0-9]+:	8f e9 50 01 64 01 f1[ 	]+tzmsk  ebp,DWORD PTR \[rcx\+rax\*1\-0xf\]
[ 	]*[a-f0-9]+:	67 8f c9 20 01 27[ 	]+tzmsk  r11d,DWORD PTR \[r15d\]
[ 	]*[a-f0-9]+:	67 8f e9 e8 01 24 dd ad de 00 00[ 	]+tzmsk  rdx,QWORD PTR \[ebx\*8\+0xdead\]
[ 	]*[a-f0-9]+:	67 8f e9 80 01 24 15 f8 ff ff ff[ 	]+tzmsk  r15,QWORD PTR \[edx\*1\-0x8\]
[ 	]*[a-f0-9]+:	8f e9 f8 01 e4[ 	]+tzmsk  rax,rsp
[ 	]*[a-f0-9]+:	67 8f c9 b8 01 21[ 	]+tzmsk  r8,QWORD PTR \[r9d\]
[ 	]*[a-f0-9]+:	8f e9 98 01 e0[ 	]+tzmsk  r12,rax
[ 	]*[a-f0-9]+:	8f c9 d0 01 e7[ 	]+tzmsk  rbp,r15
[ 	]*[a-f0-9]+:	8f 89 98 01 24 c9[ 	]+tzmsk  r12,QWORD PTR \[r9\+r9\*8\]
[ 	]*[a-f0-9]+:	67 8f e9 90 01 24 9f[ 	]+tzmsk  r13,QWORD PTR \[edi\+ebx\*4\]
[ 	]*[a-f0-9]+:	8f e9 c0 01 e7[ 	]+tzmsk  rdi,rdi
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 23[ 	]+tzmsk  rax,QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	8f e9 d8 01 26[ 	]+tzmsk  rsp,QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	8f c9 f0 01 a0 02 35 ff ff[ 	]+tzmsk  rcx,QWORD PTR \[r8\-0xcafe\]
[ 	]*[a-f0-9]+:	67 8f c9 88 01 a4 02 98 3c 00 00[ 	]+tzmsk  r14,QWORD PTR \[r10d\+eax\*1\+0x3c98\]
[ 	]*[a-f0-9]+:	67 8f c9 80 01 23[ 	]+tzmsk  r15,QWORD PTR \[r11d\]
[ 	]*[a-f0-9]+:	8f e9 c8 01 e6[ 	]+tzmsk  rsi,rsi
[ 	]*[a-f0-9]+:	8f a9 b0 01 24 05 53 21 ff ff[ 	]+tzmsk  r9,QWORD PTR \[r8\*1\-0xdead\]
#pass
