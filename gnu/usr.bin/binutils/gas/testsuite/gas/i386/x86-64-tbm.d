#objdump: -dw
#name: x86-64 TBM

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f 6a 78 10 f8 00 00 00 00 	bextr  \$0x0,%eax,%r15d
[ 	]*[a-f0-9]+:	8f 4a 78 10 d7 f1 4d 00 00 	bextr  \$0x4df1,%r15d,%r10d
[ 	]*[a-f0-9]+:	8f 4a 78 10 f5 92 5e a5 2d 	bextr  \$0x2da55e92,%r13d,%r14d
[ 	]*[a-f0-9]+:	67 8f 8a 78 10 44 7d 06 ff ff ff 7f 	bextr  \$0x7fffffff,0x6\(%r13d,%r15d,2\),%eax
[ 	]*[a-f0-9]+:	8f ca 78 10 eb 61 f7 1e 25 	bextr  \$0x251ef761,%r11d,%ebp
[ 	]*[a-f0-9]+:	8f 6a 78 10 3c d7 39 2b 00 00 	bextr  \$0x2b39,\(%rdi,%rdx,8\),%r15d
[ 	]*[a-f0-9]+:	8f 2a 78 10 0c 35 ad de 00 00 92 00 00 00 	bextr  \$0x92,0xdead\(,%r14,1\),%r9d
[ 	]*[a-f0-9]+:	8f ca 78 10 75 00 87 68 00 00 	bextr  \$0x6887,0x0\(%r13\),%esi
[ 	]*[a-f0-9]+:	67 8f ca 78 10 09 0d 00 00 00 	bextr  \$0xd,\(%r9d\),%ecx
[ 	]*[a-f0-9]+:	8f ea 78 10 1c 05 d8 40 00 00 2b 00 00 00 	bextr  \$0x2b,0x40d8\(,%rax,1\),%ebx
[ 	]*[a-f0-9]+:	8f 4a 78 10 00 2d ea 00 00 	bextr  \$0xea2d,\(%r8\),%r8d
[ 	]*[a-f0-9]+:	67 8f 4a 78 10 65 00 6c 00 00 00 	bextr  \$0x6c,0x0\(%r13d\),%r12d
[ 	]*[a-f0-9]+:	8f 6a 78 10 1c 0d 8f 8c 00 00 3b 9e 00 00 	bextr  \$0x9e3b,0x8c8f\(,%rcx,1\),%r11d
[ 	]*[a-f0-9]+:	67 8f ca 78 10 24 02 0f 00 00 00 	bextr  \$0xf,\(%r10d,%eax,1\),%esp
[ 	]*[a-f0-9]+:	67 8f aa 78 10 3c cd 00 00 00 00 ad de 00 00 	bextr  \$0xdead,0x0\(,%r9d,8\),%edi
[ 	]*[a-f0-9]+:	8f ca 78 10 c0 fe ca 00 00 	bextr  \$0xcafe,%r8d,%eax
[ 	]*[a-f0-9]+:	8f 4a f8 10 81 bc 10 00 00 b9 3b 26 7d 	bextr  \$0x7d263bb9,0x10bc\(%r9\),%r8
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 3c 65 00 00 00 00 67 00 00 00 	bextr  \$0x67,0x0\(,%r12d,2\),%r15
[ 	]*[a-f0-9]+:	8f ea f8 10 c0 00 00 00 00 	bextr  \$0x0,%rax,%rax
[ 	]*[a-f0-9]+:	67 8f ea f8 10 26 9b 53 00 00 	bextr  \$0x539b,\(%esi\),%rsp
[ 	]*[a-f0-9]+:	8f ca f8 10 08 ff ff ff 7f 	bextr  \$0x7fffffff,\(%r8\),%rcx
[ 	]*[a-f0-9]+:	67 8f ea f8 10 04 3d ff ff ff 3f 01 00 00 00 	bextr  \$0x1,0x3fffffff\(,%edi,1\),%rax
[ 	]*[a-f0-9]+:	67 8f 8a f8 10 b4 30 84 dd ff ff 9e 00 00 00 	bextr  \$0x9e,-0x227c\(%r8d,%r14d,1\),%rsi
[ 	]*[a-f0-9]+:	8f ca f8 10 c7 64 c4 a6 02 	bextr  \$0x2a6c464,%r15,%rax
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 4c 1f 02 04 00 00 00 	bextr  \$0x4,0x2\(%edi,%r11d,1\),%r9
[ 	]*[a-f0-9]+:	8f ea f8 10 ef 02 00 00 00 	bextr  \$0x2,%rdi,%rbp
[ 	]*[a-f0-9]+:	67 8f ca f8 10 14 16 fb 7e 1e 78 	bextr  \$0x781e7efb,\(%r14d,%edx,1\),%rdx
[ 	]*[a-f0-9]+:	8f 0a f8 10 ac 2b 68 db 00 00 39 40 cb 70 	bextr  \$0x70cb4039,0xdb68\(%r11,%r13,1\),%r13
[ 	]*[a-f0-9]+:	8f 4a f8 10 16 73 13 00 00 	bextr  \$0x1373,\(%r14\),%r10
[ 	]*[a-f0-9]+:	67 8f 2a f8 10 3c af 6d 55 00 00 	bextr  \$0x556d,\(%edi,%r13d,4\),%r15
[ 	]*[a-f0-9]+:	8f 4a f8 10 11 00 00 00 00 	bextr  \$0x0,\(%r9\),%r10
[ 	]*[a-f0-9]+:	8f 6a f8 10 1f ef ee ee 7b 	bextr  \$0x7beeeeef,\(%rdi\),%r11
[ 	]*[a-f0-9]+:	8f e9 00 01 cc       	blcfill %esp,%r15d
[ 	]*[a-f0-9]+:	8f a9 68 01 0c a6    	blcfill \(%rsi,%r12,4\),%edx
[ 	]*[a-f0-9]+:	67 8f e9 08 01 08    	blcfill \(%eax\),%r14d
[ 	]*[a-f0-9]+:	8f a9 50 01 0c ad 00 00 00 00 	blcfill 0x0\(,%r13,4\),%ebp
[ 	]*[a-f0-9]+:	67 8f c9 78 01 0e    	blcfill \(%r14d\),%eax
[ 	]*[a-f0-9]+:	8f c9 30 01 0b       	blcfill \(%r11\),%r9d
[ 	]*[a-f0-9]+:	8f a9 10 01 0c 45 ad de 00 00 	blcfill 0xdead\(,%r8,2\),%r13d
[ 	]*[a-f0-9]+:	8f c9 00 01 cf       	blcfill %r15d,%r15d
[ 	]*[a-f0-9]+:	8f c9 40 01 ce       	blcfill %r14d,%edi
[ 	]*[a-f0-9]+:	8f e9 20 01 c8       	blcfill %eax,%r11d
[ 	]*[a-f0-9]+:	8f c9 18 01 c9       	blcfill %r9d,%r12d
[ 	]*[a-f0-9]+:	67 8f c9 60 01 4d 67 	blcfill 0x67\(%r13d\),%ebx
[ 	]*[a-f0-9]+:	67 8f e9 00 01 0b    	blcfill \(%ebx\),%r15d
[ 	]*[a-f0-9]+:	67 8f a9 08 01 4c 19 0b 	blcfill 0xb\(%ecx,%r11d,1\),%r14d
[ 	]*[a-f0-9]+:	8f c9 78 01 8d 4a ff ff ff 	blcfill -0xb6\(%r13\),%eax
[ 	]*[a-f0-9]+:	8f c9 48 01 09       	blcfill \(%r9\),%esi
[ 	]*[a-f0-9]+:	8f c9 f8 01 cf       	blcfill %r15,%rax
[ 	]*[a-f0-9]+:	8f c9 a0 01 cd       	blcfill %r13,%r11
[ 	]*[a-f0-9]+:	8f c9 e0 01 c8       	blcfill %r8,%rbx
[ 	]*[a-f0-9]+:	67 8f c9 80 01 0f    	blcfill \(%r15d\),%r15
[ 	]*[a-f0-9]+:	67 8f c9 88 01 4d 00 	blcfill 0x0\(%r13d\),%r14
[ 	]*[a-f0-9]+:	8f e9 b0 01 c8       	blcfill %rax,%r9
[ 	]*[a-f0-9]+:	8f 89 e8 01 4c 24 0a 	blcfill 0xa\(%r12,%r12,1\),%rdx
[ 	]*[a-f0-9]+:	8f c9 98 01 ce       	blcfill %r14,%r12
[ 	]*[a-f0-9]+:	8f e9 a8 01 cf       	blcfill %rdi,%r10
[ 	]*[a-f0-9]+:	67 8f c9 90 01 0b    	blcfill \(%r11d\),%r13
[ 	]*[a-f0-9]+:	67 8f e9 b8 01 0c 15 25 c6 ff ff 	blcfill -0x39db\(,%edx,1\),%r8
[ 	]*[a-f0-9]+:	8f c9 d8 01 0c 34    	blcfill \(%r12,%rsi,1\),%rsp
[ 	]*[a-f0-9]+:	67 8f 89 b8 01 4c 6d 00 	blcfill 0x0\(%r13d,%r13d,2\),%r8
[ 	]*[a-f0-9]+:	8f e9 d0 01 08       	blcfill \(%rax\),%rbp
[ 	]*[a-f0-9]+:	8f c9 80 01 09       	blcfill \(%r9\),%r15
[ 	]*[a-f0-9]+:	8f c9 f0 01 cb       	blcfill %r11,%rcx
[ 	]*[a-f0-9]+:	8f c9 78 02 f7       	blci   %r15d,%eax
[ 	]*[a-f0-9]+:	8f e9 00 02 32       	blci   \(%rdx\),%r15d
[ 	]*[a-f0-9]+:	8f e9 28 02 f0       	blci   %eax,%r10d
[ 	]*[a-f0-9]+:	67 8f e9 38 02 37    	blci   \(%edi\),%r8d
[ 	]*[a-f0-9]+:	67 8f c9 68 02 75 00 	blci   0x0\(%r13d\),%edx
[ 	]*[a-f0-9]+:	67 8f e9 20 02 32    	blci   \(%edx\),%r11d
[ 	]*[a-f0-9]+:	67 8f e9 18 02 34 05 37 09 00 00 	blci   0x937\(,%eax,1\),%r12d
[ 	]*[a-f0-9]+:	8f c9 70 02 31       	blci   \(%r9\),%ecx
[ 	]*[a-f0-9]+:	67 8f c9 58 02 31    	blci   \(%r9d\),%esp
[ 	]*[a-f0-9]+:	8f e9 48 02 f2       	blci   %edx,%esi
[ 	]*[a-f0-9]+:	8f e9 08 02 f5       	blci   %ebp,%r14d
[ 	]*[a-f0-9]+:	8f e9 78 02 f3       	blci   %ebx,%eax
[ 	]*[a-f0-9]+:	8f e9 38 02 30       	blci   \(%rax\),%r8d
[ 	]*[a-f0-9]+:	67 8f a9 40 02 34 75 00 00 00 00 	blci   0x0\(,%r14d,2\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 02 33       	blci   \(%rbx\),%eax
[ 	]*[a-f0-9]+:	67 8f 89 30 02 b4 31 31 a3 4c 43 	blci   0x434ca331\(%r9d,%r14d,1\),%r9d
[ 	]*[a-f0-9]+:	67 8f e9 a0 02 33    	blci   \(%ebx\),%r11
[ 	]*[a-f0-9]+:	8f c9 f8 02 37       	blci   \(%r15\),%rax
[ 	]*[a-f0-9]+:	67 8f c9 80 02 34 dc 	blci   \(%r12d,%ebx,8\),%r15
[ 	]*[a-f0-9]+:	8f c9 d0 02 f7       	blci   %r15,%rbp
[ 	]*[a-f0-9]+:	67 8f e9 d8 02 34 33 	blci   \(%ebx,%esi,1\),%rsp
[ 	]*[a-f0-9]+:	8f c9 f0 02 f4       	blci   %r12,%rcx
[ 	]*[a-f0-9]+:	8f c9 c0 02 31       	blci   \(%r9\),%rdi
[ 	]*[a-f0-9]+:	67 8f c9 e0 02 34 3c 	blci   \(%r12d,%edi,1\),%rbx
[ 	]*[a-f0-9]+:	8f e9 80 02 34 d5 19 5b 00 00 	blci   0x5b19\(,%rdx,8\),%r15
[ 	]*[a-f0-9]+:	67 8f e9 a8 02 34 c5 00 00 00 00 	blci   0x0\(,%eax,8\),%r10
[ 	]*[a-f0-9]+:	8f e9 b8 02 33       	blci   \(%rbx\),%r8
[ 	]*[a-f0-9]+:	67 8f e9 b0 02 b4 50 0b ff ff ff 	blci   -0xf5\(%eax,%edx,2\),%r9
[ 	]*[a-f0-9]+:	8f c9 88 02 75 00    	blci   0x0\(%r13\),%r14
[ 	]*[a-f0-9]+:	8f e9 f8 02 f5       	blci   %rbp,%rax
[ 	]*[a-f0-9]+:	67 8f e9 90 02 30    	blci   \(%eax\),%r13
[ 	]*[a-f0-9]+:	8f c9 e8 02 34 24    	blci   \(%r12\),%rdx
[ 	]*[a-f0-9]+:	67 8f c9 00 01 2c c6 	blcic  \(%r14d,%eax,8\),%r15d
[ 	]*[a-f0-9]+:	8f c9 78 01 ef       	blcic  %r15d,%eax
[ 	]*[a-f0-9]+:	8f c9 38 01 29       	blcic  \(%r9\),%r8d
[ 	]*[a-f0-9]+:	8f c9 30 01 2c 59    	blcic  \(%r9,%rbx,2\),%r9d
[ 	]*[a-f0-9]+:	67 8f e9 48 01 2b    	blcic  \(%ebx\),%esi
[ 	]*[a-f0-9]+:	67 8f e9 50 01 2c 05 fe ff ff ff 	blcic  -0x2\(,%eax,1\),%ebp
[ 	]*[a-f0-9]+:	8f e9 60 01 28       	blcic  \(%rax\),%ebx
[ 	]*[a-f0-9]+:	8f c9 40 01 2b       	blcic  \(%r11\),%edi
[ 	]*[a-f0-9]+:	8f e9 20 01 e8       	blcic  %eax,%r11d
[ 	]*[a-f0-9]+:	8f c9 18 01 2e       	blcic  \(%r14\),%r12d
[ 	]*[a-f0-9]+:	8f c9 78 01 eb       	blcic  %r11d,%eax
[ 	]*[a-f0-9]+:	8f a9 00 01 2c 1d a7 d0 1a 14 	blcic  0x141ad0a7\(,%r11,1\),%r15d
[ 	]*[a-f0-9]+:	8f a9 10 01 2c 88    	blcic  \(%rax,%r9,4\),%r13d
[ 	]*[a-f0-9]+:	8f e9 00 01 2b       	blcic  \(%rbx\),%r15d
[ 	]*[a-f0-9]+:	67 8f 89 28 01 2c 3f 	blcic  \(%r15d,%r15d,1\),%r10d
[ 	]*[a-f0-9]+:	67 8f c9 68 01 29    	blcic  \(%r9d\),%edx
[ 	]*[a-f0-9]+:	67 8f a9 f0 01 2c 2d b3 cb d3 59 	blcic  0x59d3cbb3\(,%r13d,1\),%rcx
[ 	]*[a-f0-9]+:	8f c9 f8 01 ee       	blcic  %r14,%rax
[ 	]*[a-f0-9]+:	67 8f c9 80 01 2c 24 	blcic  \(%r12d\),%r15
[ 	]*[a-f0-9]+:	8f e9 88 01 e8       	blcic  %rax,%r14
[ 	]*[a-f0-9]+:	8f c9 d0 01 ef       	blcic  %r15,%rbp
[ 	]*[a-f0-9]+:	8f e9 d8 01 2b       	blcic  \(%rbx\),%rsp
[ 	]*[a-f0-9]+:	8f e9 e8 01 eb       	blcic  %rbx,%rdx
[ 	]*[a-f0-9]+:	8f c9 c0 01 e8       	blcic  %r8,%rdi
[ 	]*[a-f0-9]+:	8f c9 c8 01 29       	blcic  \(%r9\),%rsi
[ 	]*[a-f0-9]+:	8f e9 c0 01 2c c5 db db 00 00 	blcic  0xdbdb\(,%rax,8\),%rdi
[ 	]*[a-f0-9]+:	8f c9 e0 01 ea       	blcic  %r10,%rbx
[ 	]*[a-f0-9]+:	67 8f e9 a0 01 2b    	blcic  \(%ebx\),%r11
[ 	]*[a-f0-9]+:	8f c9 b0 01 ed       	blcic  %r13,%r9
[ 	]*[a-f0-9]+:	8f c9 f8 01 28       	blcic  \(%r8\),%rax
[ 	]*[a-f0-9]+:	8f 89 98 01 ac 12 ad de 00 00 	blcic  0xdead\(%r10,%r10,1\),%r12
[ 	]*[a-f0-9]+:	67 8f e9 f0 01 2c 02 	blcic  \(%edx,%eax,1\),%rcx
[ 	]*[a-f0-9]+:	67 8f e9 00 02 09    	blcmsk \(%ecx\),%r15d
[ 	]*[a-f0-9]+:	8f e9 78 02 cd       	blcmsk %ebp,%eax
[ 	]*[a-f0-9]+:	67 8f e9 40 02 0b    	blcmsk \(%ebx\),%edi
[ 	]*[a-f0-9]+:	8f e9 68 02 c8       	blcmsk %eax,%edx
[ 	]*[a-f0-9]+:	8f a9 10 02 0c d5 00 00 00 00 	blcmsk 0x0\(,%r10,8\),%r13d
[ 	]*[a-f0-9]+:	8f c9 30 02 09       	blcmsk \(%r9\),%r9d
[ 	]*[a-f0-9]+:	8f c9 18 02 0a       	blcmsk \(%r10\),%r12d
[ 	]*[a-f0-9]+:	8f e9 60 02 c9       	blcmsk %ecx,%ebx
[ 	]*[a-f0-9]+:	67 8f e9 78 02 0a    	blcmsk \(%edx\),%eax
[ 	]*[a-f0-9]+:	8f e9 20 02 ce       	blcmsk %esi,%r11d
[ 	]*[a-f0-9]+:	8f a9 00 02 0c b5 00 00 00 00 	blcmsk 0x0\(,%r14,4\),%r15d
[ 	]*[a-f0-9]+:	8f c9 78 02 cf       	blcmsk %r15d,%eax
[ 	]*[a-f0-9]+:	67 8f c9 08 02 8e 5f f3 00 00 	blcmsk 0xf35f\(%r14d\),%r14d
[ 	]*[a-f0-9]+:	67 8f c9 38 02 0c 30 	blcmsk \(%r8d,%esi,1\),%r8d
[ 	]*[a-f0-9]+:	8f c9 58 02 0c 14    	blcmsk \(%r12,%rdx,1\),%esp
[ 	]*[a-f0-9]+:	67 8f c9 28 02 08    	blcmsk \(%r8d\),%r10d
[ 	]*[a-f0-9]+:	67 8f a9 98 02 0c 2d 00 00 00 00 	blcmsk 0x0\(,%r13d,1\),%r12
[ 	]*[a-f0-9]+:	8f c9 e0 02 cf       	blcmsk %r15,%rbx
[ 	]*[a-f0-9]+:	8f e9 80 02 c8       	blcmsk %rax,%r15
[ 	]*[a-f0-9]+:	67 8f a9 b8 02 0c 0d 03 00 00 00 	blcmsk 0x3\(,%r9d,1\),%r8
[ 	]*[a-f0-9]+:	8f 89 d0 02 8c 79 02 35 ff ff 	blcmsk -0xcafe\(%r9,%r15,2\),%rbp
[ 	]*[a-f0-9]+:	8f c9 d8 02 4d 00    	blcmsk 0x0\(%r13\),%rsp
[ 	]*[a-f0-9]+:	8f e9 f8 02 0a       	blcmsk \(%rdx\),%rax
[ 	]*[a-f0-9]+:	8f c9 90 02 0c 24    	blcmsk \(%r12\),%r13
[ 	]*[a-f0-9]+:	8f e9 e8 02 0c d5 f9 ff ff ff 	blcmsk -0x7\(,%rdx,8\),%rdx
[ 	]*[a-f0-9]+:	8f c9 88 02 0b       	blcmsk \(%r11\),%r14
[ 	]*[a-f0-9]+:	8f c9 b0 02 ce       	blcmsk %r14,%r9
[ 	]*[a-f0-9]+:	8f e9 a0 02 09       	blcmsk \(%rcx\),%r11
[ 	]*[a-f0-9]+:	67 8f c9 f8 02 0e    	blcmsk \(%r14d\),%rax
[ 	]*[a-f0-9]+:	8f e9 c0 02 0c c5 00 00 00 00 	blcmsk 0x0\(,%rax,8\),%rdi
[ 	]*[a-f0-9]+:	67 8f c9 90 02 0f    	blcmsk \(%r15d\),%r13
[ 	]*[a-f0-9]+:	67 8f e9 88 02 0c 33 	blcmsk \(%ebx,%esi,1\),%r14
[ 	]*[a-f0-9]+:	8f e9 00 01 18       	blcs   \(%rax\),%r15d
[ 	]*[a-f0-9]+:	67 8f a9 38 01 1c 05 01 00 00 00 	blcs   0x1\(,%r8d,1\),%r8d
[ 	]*[a-f0-9]+:	8f c9 70 01 da       	blcs   %r10d,%ecx
[ 	]*[a-f0-9]+:	8f c9 28 01 df       	blcs   %r15d,%r10d
[ 	]*[a-f0-9]+:	8f c9 78 01 db       	blcs   %r11d,%eax
[ 	]*[a-f0-9]+:	67 8f e9 40 01 99 9b dc 68 81 	blcs   -0x7e972365\(%ecx\),%edi
[ 	]*[a-f0-9]+:	67 8f e9 08 01 1e    	blcs   \(%esi\),%r14d
[ 	]*[a-f0-9]+:	8f c9 20 01 5a fd    	blcs   -0x3\(%r10\),%r11d
[ 	]*[a-f0-9]+:	8f e9 58 01 1f       	blcs   \(%rdi\),%esp
[ 	]*[a-f0-9]+:	67 8f c9 60 01 1f    	blcs   \(%r15d\),%ebx
[ 	]*[a-f0-9]+:	8f c9 10 01 1c b1    	blcs   \(%r9,%rsi,4\),%r13d
[ 	]*[a-f0-9]+:	8f c9 30 01 1c 19    	blcs   \(%r9,%rbx,1\),%r9d
[ 	]*[a-f0-9]+:	67 8f e9 00 01 1c 08 	blcs   \(%eax,%ecx,1\),%r15d
[ 	]*[a-f0-9]+:	8f e9 48 01 db       	blcs   %ebx,%esi
[ 	]*[a-f0-9]+:	8f e9 78 01 de       	blcs   %esi,%eax
[ 	]*[a-f0-9]+:	8f e9 18 01 df       	blcs   %edi,%r12d
[ 	]*[a-f0-9]+:	8f e9 f8 01 df       	blcs   %rdi,%rax
[ 	]*[a-f0-9]+:	8f e9 98 01 18       	blcs   \(%rax\),%r12
[ 	]*[a-f0-9]+:	8f c9 80 01 df       	blcs   %r15,%r15
[ 	]*[a-f0-9]+:	8f c9 f0 01 da       	blcs   %r10,%rcx
[ 	]*[a-f0-9]+:	67 8f e9 90 01 18    	blcs   \(%eax\),%r13
[ 	]*[a-f0-9]+:	8f e9 b8 01 d8       	blcs   %rax,%r8
[ 	]*[a-f0-9]+:	67 8f e9 c0 01 5a ff 	blcs   -0x1\(%edx\),%rdi
[ 	]*[a-f0-9]+:	8f e9 a0 01 db       	blcs   %rbx,%r11
[ 	]*[a-f0-9]+:	67 8f e9 d8 01 1c 45 00 00 00 00 	blcs   0x0\(,%eax,2\),%rsp
[ 	]*[a-f0-9]+:	8f 89 a8 01 1c 29    	blcs   \(%r9,%r13,1\),%r10
[ 	]*[a-f0-9]+:	67 8f a9 88 01 1c 05 cf 1d 00 00 	blcs   0x1dcf\(,%r8d,1\),%r14
[ 	]*[a-f0-9]+:	67 8f a9 80 01 1c bd 00 00 00 00 	blcs   0x0\(,%r15d,4\),%r15
[ 	]*[a-f0-9]+:	8f c9 d0 01 19       	blcs   \(%r9\),%rbp
[ 	]*[a-f0-9]+:	67 8f c9 e8 01 5c 05 00 	blcs   0x0\(%r13d,%eax,1\),%rdx
[ 	]*[a-f0-9]+:	8f c9 d8 01 dc       	blcs   %r12,%rsp
[ 	]*[a-f0-9]+:	8f e9 e0 01 1f       	blcs   \(%rdi\),%rbx
[ 	]*[a-f0-9]+:	67 8f e9 68 01 16    	blsfill \(%esi\),%edx
[ 	]*[a-f0-9]+:	8f c9 78 01 11       	blsfill \(%r9\),%eax
[ 	]*[a-f0-9]+:	67 8f e9 00 01 13    	blsfill \(%ebx\),%r15d
[ 	]*[a-f0-9]+:	8f e9 20 01 d0       	blsfill %eax,%r11d
[ 	]*[a-f0-9]+:	8f c9 38 01 14 24    	blsfill \(%r12\),%r8d
[ 	]*[a-f0-9]+:	67 8f a9 00 01 14 0d 7e aa ff ff 	blsfill -0x5582\(,%r9d,1\),%r15d
[ 	]*[a-f0-9]+:	8f e9 78 01 d4       	blsfill %esp,%eax
[ 	]*[a-f0-9]+:	67 8f a9 50 01 14 65 00 00 00 00 	blsfill 0x0\(,%r12d,2\),%ebp
[ 	]*[a-f0-9]+:	67 8f c9 60 01 10    	blsfill \(%r8d\),%ebx
[ 	]*[a-f0-9]+:	67 8f e9 58 01 10    	blsfill \(%eax\),%esp
[ 	]*[a-f0-9]+:	8f a9 18 01 14 1d 03 4f 00 00 	blsfill 0x4f03\(,%r11,1\),%r12d
[ 	]*[a-f0-9]+:	67 8f a9 78 01 14 15 0f 00 00 00 	blsfill 0xf\(,%r10d,1\),%eax
[ 	]*[a-f0-9]+:	67 8f c9 40 01 17    	blsfill \(%r15d\),%edi
[ 	]*[a-f0-9]+:	8f e9 70 01 14 35 8f 22 00 00 	blsfill 0x228f\(,%rsi,1\),%ecx
[ 	]*[a-f0-9]+:	67 8f e9 48 01 11    	blsfill \(%ecx\),%esi
[ 	]*[a-f0-9]+:	8f c9 10 01 d0       	blsfill %r8d,%r13d
[ 	]*[a-f0-9]+:	67 8f e9 80 01 14 85 f4 ff ff ff 	blsfill -0xc\(,%eax,4\),%r15
[ 	]*[a-f0-9]+:	8f e9 98 01 d0       	blsfill %rax,%r12
[ 	]*[a-f0-9]+:	8f e9 f8 01 d2       	blsfill %rdx,%rax
[ 	]*[a-f0-9]+:	8f c9 d0 01 11       	blsfill \(%r9\),%rbp
[ 	]*[a-f0-9]+:	67 8f e9 e0 01 17    	blsfill \(%edi\),%rbx
[ 	]*[a-f0-9]+:	8f c9 b0 01 d7       	blsfill %r15,%r9
[ 	]*[a-f0-9]+:	8f e9 d8 01 d3       	blsfill %rbx,%rsp
[ 	]*[a-f0-9]+:	8f c9 f8 01 17       	blsfill \(%r15\),%rax
[ 	]*[a-f0-9]+:	67 8f e9 a8 01 94 3f b9 56 00 00 	blsfill 0x56b9\(%edi,%edi,1\),%r10
[ 	]*[a-f0-9]+:	67 8f c9 f0 01 94 b4 2f d4 ff ff 	blsfill -0x2bd1\(%r12d,%esi,4\),%rcx
[ 	]*[a-f0-9]+:	8f c9 d8 01 13       	blsfill \(%r11\),%rsp
[ 	]*[a-f0-9]+:	8f c9 b8 01 d5       	blsfill %r13,%r8
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 14 43 	blsfill \(%ebx,%eax,2\),%rax
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 13    	blsfill \(%ebx\),%rax
[ 	]*[a-f0-9]+:	8f e9 a0 01 14 13    	blsfill \(%rbx,%rdx,1\),%r11
[ 	]*[a-f0-9]+:	8f c9 c8 01 95 dc 2f 00 00 	blsfill 0x2fdc\(%r13\),%rsi
[ 	]*[a-f0-9]+:	8f c9 00 01 f3       	blsic  %r11d,%r15d
[ 	]*[a-f0-9]+:	8f e9 50 01 34 35 61 86 ff ff 	blsic  -0x799f\(,%rsi,1\),%ebp
[ 	]*[a-f0-9]+:	8f c9 78 01 f7       	blsic  %r15d,%eax
[ 	]*[a-f0-9]+:	8f a9 70 01 34 10    	blsic  \(%rax,%r10,1\),%ecx
[ 	]*[a-f0-9]+:	8f e9 28 01 f0       	blsic  %eax,%r10d
[ 	]*[a-f0-9]+:	67 8f c9 30 01 75 00 	blsic  0x0\(%r13d\),%r9d
[ 	]*[a-f0-9]+:	8f c9 60 01 31       	blsic  \(%r9\),%ebx
[ 	]*[a-f0-9]+:	67 8f e9 58 01 33    	blsic  \(%ebx\),%esp
[ 	]*[a-f0-9]+:	67 8f c9 20 01 34 24 	blsic  \(%r12d\),%r11d
[ 	]*[a-f0-9]+:	8f e9 68 01 34 3d fe bc 00 00 	blsic  0xbcfe\(,%rdi,1\),%edx
[ 	]*[a-f0-9]+:	67 8f c9 40 01 36    	blsic  \(%r14d\),%edi
[ 	]*[a-f0-9]+:	67 8f a9 00 01 34 2d ec 78 00 00 	blsic  0x78ec\(,%r13d,1\),%r15d
[ 	]*[a-f0-9]+:	67 8f c9 48 01 33    	blsic  \(%r11d\),%esi
[ 	]*[a-f0-9]+:	8f c9 08 01 32       	blsic  \(%r10\),%r14d
[ 	]*[a-f0-9]+:	67 8f c9 00 01 31    	blsic  \(%r9d\),%r15d
[ 	]*[a-f0-9]+:	8f c9 00 01 f2       	blsic  %r10d,%r15d
[ 	]*[a-f0-9]+:	8f c9 f8 01 f7       	blsic  %r15,%rax
[ 	]*[a-f0-9]+:	8f e9 b0 01 34 05 67 00 00 00 	blsic  0x67\(,%rax,1\),%r9
[ 	]*[a-f0-9]+:	67 8f 89 e8 01 34 20 	blsic  \(%r8d,%r12d,1\),%rdx
[ 	]*[a-f0-9]+:	67 8f c9 80 01 37    	blsic  \(%r15d\),%r15
[ 	]*[a-f0-9]+:	8f c9 f0 01 f1       	blsic  %r9,%rcx
[ 	]*[a-f0-9]+:	8f c9 c0 01 f2       	blsic  %r10,%rdi
[ 	]*[a-f0-9]+:	8f a9 e0 01 34 05 ff ff ff 3f 	blsic  0x3fffffff\(,%r8,1\),%rbx
[ 	]*[a-f0-9]+:	8f e9 80 01 f2       	blsic  %rdx,%r15
[ 	]*[a-f0-9]+:	8f e9 c8 01 30       	blsic  \(%rax\),%rsi
[ 	]*[a-f0-9]+:	67 8f c9 f8 01 37    	blsic  \(%r15d\),%rax
[ 	]*[a-f0-9]+:	8f e9 80 01 33       	blsic  \(%rbx\),%r15
[ 	]*[a-f0-9]+:	8f e9 b8 01 f0       	blsic  %rax,%r8
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 33    	blsic  \(%ebx\),%rax
[ 	]*[a-f0-9]+:	8f e9 88 01 f1       	blsic  %rcx,%r14
[ 	]*[a-f0-9]+:	67 8f c9 c8 01 34 07 	blsic  \(%r15d,%eax,1\),%rsi
[ 	]*[a-f0-9]+:	8f c9 98 01 f5       	blsic  %r13,%r12
[ 	]*[a-f0-9]+:	8f e9 00 01 7e fd    	t1mskc -0x3\(%rsi\),%r15d
[ 	]*[a-f0-9]+:	8f c9 18 01 ff       	t1mskc %r15d,%r12d
[ 	]*[a-f0-9]+:	8f c9 30 01 3c 24    	t1mskc \(%r12\),%r9d
[ 	]*[a-f0-9]+:	8f e9 78 01 fe       	t1mskc %esi,%eax
[ 	]*[a-f0-9]+:	67 8f c9 58 01 7a fe 	t1mskc -0x2\(%r10d\),%esp
[ 	]*[a-f0-9]+:	67 8f e9 10 01 3c 45 00 00 00 00 	t1mskc 0x0\(,%eax,2\),%r13d
[ 	]*[a-f0-9]+:	8f e9 48 01 f8       	t1mskc %eax,%esi
[ 	]*[a-f0-9]+:	67 8f c9 78 01 3c 24 	t1mskc \(%r12d\),%eax
[ 	]*[a-f0-9]+:	8f e9 28 01 3c 1d 9c f5 00 00 	t1mskc 0xf59c\(,%rbx,1\),%r10d
[ 	]*[a-f0-9]+:	67 8f e9 20 01 3c 85 00 00 00 00 	t1mskc 0x0\(,%eax,4\),%r11d
[ 	]*[a-f0-9]+:	67 8f e9 38 01 3b    	t1mskc \(%ebx\),%r8d
[ 	]*[a-f0-9]+:	8f e9 60 01 ff       	t1mskc %edi,%ebx
[ 	]*[a-f0-9]+:	67 8f e9 08 01 3a    	t1mskc \(%edx\),%r14d
[ 	]*[a-f0-9]+:	67 8f c9 00 01 3b    	t1mskc \(%r11d\),%r15d
[ 	]*[a-f0-9]+:	67 8f e9 70 01 3e    	t1mskc \(%esi\),%ecx
[ 	]*[a-f0-9]+:	8f 89 40 01 3c 29    	t1mskc \(%r9,%r13,1\),%edi
[ 	]*[a-f0-9]+:	8f c9 d8 01 be ff ff ff 3f 	t1mskc 0x3fffffff\(%r14\),%rsp
[ 	]*[a-f0-9]+:	8f e9 f8 01 f8       	t1mskc %rax,%rax
[ 	]*[a-f0-9]+:	8f c9 e0 01 38       	t1mskc \(%r8\),%rbx
[ 	]*[a-f0-9]+:	67 8f c9 c0 01 3c 3c 	t1mskc \(%r12d,%edi,1\),%rdi
[ 	]*[a-f0-9]+:	8f c9 f0 01 fb       	t1mskc %r11,%rcx
[ 	]*[a-f0-9]+:	8f c9 88 01 7d 00    	t1mskc 0x0\(%r13\),%r14
[ 	]*[a-f0-9]+:	67 8f e9 e8 01 3c c5 ad de 00 00 	t1mskc 0xdead\(,%eax,8\),%rdx
[ 	]*[a-f0-9]+:	8f c9 80 01 ff       	t1mskc %r15,%r15
[ 	]*[a-f0-9]+:	8f c9 d0 01 3f       	t1mskc \(%r15\),%rbp
[ 	]*[a-f0-9]+:	8f e9 b0 01 fc       	t1mskc %rsp,%r9
[ 	]*[a-f0-9]+:	8f e9 c8 01 3a       	t1mskc \(%rdx\),%rsi
[ 	]*[a-f0-9]+:	8f c9 a8 01 fa       	t1mskc %r10,%r10
[ 	]*[a-f0-9]+:	67 8f c9 90 01 39    	t1mskc \(%r9d\),%r13
[ 	]*[a-f0-9]+:	8f e9 f8 01 fb       	t1mskc %rbx,%rax
[ 	]*[a-f0-9]+:	8f c9 f8 01 39       	t1mskc \(%r9\),%rax
[ 	]*[a-f0-9]+:	67 8f c9 a8 01 38    	t1mskc \(%r8d\),%r10
[ 	]*[a-f0-9]+:	8f e9 28 01 e3       	tzmsk  %ebx,%r10d
[ 	]*[a-f0-9]+:	8f c9 78 01 21       	tzmsk  \(%r9\),%eax
[ 	]*[a-f0-9]+:	8f e9 00 01 22       	tzmsk  \(%rdx\),%r15d
[ 	]*[a-f0-9]+:	8f e9 18 01 e5       	tzmsk  %ebp,%r12d
[ 	]*[a-f0-9]+:	8f c9 10 01 e2       	tzmsk  %r10d,%r13d
[ 	]*[a-f0-9]+:	8f c9 00 01 e7       	tzmsk  %r15d,%r15d
[ 	]*[a-f0-9]+:	8f 89 60 01 a4 0b 02 35 ff ff 	tzmsk  -0xcafe\(%r11,%r9,1\),%ebx
[ 	]*[a-f0-9]+:	67 8f a9 68 01 64 2e 01 	tzmsk  0x1\(%esi,%r13d,1\),%edx
[ 	]*[a-f0-9]+:	67 8f c9 08 01 23    	tzmsk  \(%r11d\),%r14d
[ 	]*[a-f0-9]+:	67 8f a9 70 01 24 a1 	tzmsk  \(%ecx,%r12d,4\),%ecx
[ 	]*[a-f0-9]+:	67 8f e9 30 01 20    	tzmsk  \(%eax\),%r9d
[ 	]*[a-f0-9]+:	8f e9 38 01 60 fa    	tzmsk  -0x6\(%rax\),%r8d
[ 	]*[a-f0-9]+:	8f e9 48 01 e7       	tzmsk  %edi,%esi
[ 	]*[a-f0-9]+:	8f e9 00 01 e0       	tzmsk  %eax,%r15d
[ 	]*[a-f0-9]+:	8f e9 50 01 64 01 f1 	tzmsk  -0xf\(%rcx,%rax,1\),%ebp
[ 	]*[a-f0-9]+:	67 8f c9 20 01 27    	tzmsk  \(%r15d\),%r11d
[ 	]*[a-f0-9]+:	67 8f e9 e8 01 24 dd ad de 00 00 	tzmsk  0xdead\(,%ebx,8\),%rdx
[ 	]*[a-f0-9]+:	67 8f e9 80 01 24 15 f8 ff ff ff 	tzmsk  -0x8\(,%edx,1\),%r15
[ 	]*[a-f0-9]+:	8f e9 f8 01 e4       	tzmsk  %rsp,%rax
[ 	]*[a-f0-9]+:	67 8f c9 b8 01 21    	tzmsk  \(%r9d\),%r8
[ 	]*[a-f0-9]+:	8f e9 98 01 e0       	tzmsk  %rax,%r12
[ 	]*[a-f0-9]+:	8f c9 d0 01 e7       	tzmsk  %r15,%rbp
[ 	]*[a-f0-9]+:	8f 89 98 01 24 c9    	tzmsk  \(%r9,%r9,8\),%r12
[ 	]*[a-f0-9]+:	67 8f e9 90 01 24 9f 	tzmsk  \(%edi,%ebx,4\),%r13
[ 	]*[a-f0-9]+:	8f e9 c0 01 e7       	tzmsk  %rdi,%rdi
[ 	]*[a-f0-9]+:	67 8f e9 f8 01 23    	tzmsk  \(%ebx\),%rax
[ 	]*[a-f0-9]+:	8f e9 d8 01 26       	tzmsk  \(%rsi\),%rsp
[ 	]*[a-f0-9]+:	8f c9 f0 01 a0 02 35 ff ff 	tzmsk  -0xcafe\(%r8\),%rcx
[ 	]*[a-f0-9]+:	67 8f c9 88 01 a4 02 98 3c 00 00 	tzmsk  0x3c98\(%r10d,%eax,1\),%r14
[ 	]*[a-f0-9]+:	67 8f c9 80 01 23    	tzmsk  \(%r11d\),%r15
[ 	]*[a-f0-9]+:	8f e9 c8 01 e6       	tzmsk  %rsi,%rsi
[ 	]*[a-f0-9]+:	8f a9 b0 01 24 05 53 21 ff ff 	tzmsk  -0xdead\(,%r8,1\),%r9
#pass
