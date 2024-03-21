#as: -O
#objdump: -drw
#name: x86-64 optimized encoding 1 with -O

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	48 25 00 00 00 00    	and    \$0x0,%rax	2: R_X86_64_32S	foo
 +[a-f0-9]+:	25 ff ff ff 7f       	and    \$0x7fffffff,%eax
 +[a-f0-9]+:	81 e3 ff ff ff 7f    	and    \$0x7fffffff,%ebx
 +[a-f0-9]+:	41 81 e6 ff ff ff 7f 	and    \$0x7fffffff,%r14d
 +[a-f0-9]+:	48 25 00 00 00 80    	and    \$0xffffffff80000000,%rax
 +[a-f0-9]+:	48 81 e3 00 00 00 80 	and    \$0xffffffff80000000,%rbx
 +[a-f0-9]+:	49 81 e6 00 00 00 80 	and    \$0xffffffff80000000,%r14
 +[a-f0-9]+:	83 e0 7f             	and    \$0x7f,%eax
 +[a-f0-9]+:	83 e3 7f             	and    \$0x7f,%ebx
 +[a-f0-9]+:	41 83 e6 7f          	and    \$0x7f,%r14d
 +[a-f0-9]+:	48 83 e0 80          	and    \$0xffffffffffffff80,%rax
 +[a-f0-9]+:	48 83 e3 80          	and    \$0xffffffffffffff80,%rbx
 +[a-f0-9]+:	49 83 e6 80          	and    \$0xffffffffffffff80,%r14
 +[a-f0-9]+:	a9 ff ff ff 7f       	test   \$0x7fffffff,%eax
 +[a-f0-9]+:	f7 c3 ff ff ff 7f    	test   \$0x7fffffff,%ebx
 +[a-f0-9]+:	41 f7 c6 ff ff ff 7f 	test   \$0x7fffffff,%r14d
 +[a-f0-9]+:	48 a9 00 00 00 80    	test   \$0xffffffff80000000,%rax
 +[a-f0-9]+:	48 f7 c3 00 00 00 80 	test   \$0xffffffff80000000,%rbx
 +[a-f0-9]+:	49 f7 c6 00 00 00 80 	test   \$0xffffffff80000000,%r14
 +[a-f0-9]+:	48 33 06             	xor    \(%rsi\),%rax
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	31 db                	xor    %ebx,%ebx
 +[a-f0-9]+:	45 31 f6             	xor    %r14d,%r14d
 +[a-f0-9]+:	48 31 d0             	xor    %rdx,%rax
 +[a-f0-9]+:	48 31 d3             	xor    %rdx,%rbx
 +[a-f0-9]+:	49 31 d6             	xor    %rdx,%r14
 +[a-f0-9]+:	29 c0                	sub    %eax,%eax
 +[a-f0-9]+:	29 db                	sub    %ebx,%ebx
 +[a-f0-9]+:	45 29 f6             	sub    %r14d,%r14d
 +[a-f0-9]+:	48 29 d0             	sub    %rdx,%rax
 +[a-f0-9]+:	48 29 d3             	sub    %rdx,%rbx
 +[a-f0-9]+:	49 29 d6             	sub    %rdx,%r14
 +[a-f0-9]+:	48 81 20 ff ff ff 7f 	andq   \$0x7fffffff,\(%rax\)
 +[a-f0-9]+:	48 81 20 00 00 00 80 	andq   \$0xffffffff80000000,\(%rax\)
 +[a-f0-9]+:	48 f7 00 ff ff ff 7f 	testq  \$0x7fffffff,\(%rax\)
 +[a-f0-9]+:	48 f7 00 00 00 00 80 	testq  \$0xffffffff80000000,\(%rax\)
 +[a-f0-9]+:	b8 ff ff ff 7f       	mov    \$0x7fffffff,%eax
 +[a-f0-9]+:	b8 ff ff ff 7f       	mov    \$0x7fffffff,%eax
 +[a-f0-9]+:	41 b8 ff ff ff 7f    	mov    \$0x7fffffff,%r8d
 +[a-f0-9]+:	41 b8 ff ff ff 7f    	mov    \$0x7fffffff,%r8d
 +[a-f0-9]+:	b8 ff ff ff ff       	mov    \$0xffffffff,%eax
 +[a-f0-9]+:	b8 ff ff ff ff       	mov    \$0xffffffff,%eax
 +[a-f0-9]+:	41 b8 ff ff ff ff    	mov    \$0xffffffff,%r8d
 +[a-f0-9]+:	41 b8 ff ff ff ff    	mov    \$0xffffffff,%r8d
 +[a-f0-9]+:	b8 ff 03 00 00       	mov    \$0x3ff,%eax
 +[a-f0-9]+:	b8 ff 03 00 00       	mov    \$0x3ff,%eax
 +[a-f0-9]+:	48 b8 00 00 00 00 01 00 00 00 	movabs \$0x100000000,%rax
 +[a-f0-9]+:	48 b8 00 00 00 00 01 00 00 00 	movabs \$0x100000000,%rax
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	45 31 f6             	xor    %r14d,%r14d
 +[a-f0-9]+:	0f ba e0 0f          	bt     \$0xf,%eax
 +[a-f0-9]+:	66 0f ba e0 10       	bt     \$0x10,%ax
 +[a-f0-9]+:	41 0f ba e0 0f       	bt     \$0xf,%r8d
 +[a-f0-9]+:	66 41 0f ba e0 10    	bt     \$0x10,%r8w
 +[a-f0-9]+:	0f ba e0 1f          	bt     \$0x1f,%eax
 +[a-f0-9]+:	48 0f ba e0 20       	bt     \$0x20,%rax
 +[a-f0-9]+:	49 0f ba e0 1f       	bt     \$0x1f,%r8
 +[a-f0-9]+:	66 0f ba f8 0f       	btc    \$0xf,%ax
 +[a-f0-9]+:	48 0f ba f8 1f       	btc    \$0x1f,%rax
 +[a-f0-9]+:	66 0f ba f0 0f       	btr    \$0xf,%ax
 +[a-f0-9]+:	48 0f ba f0 1f       	btr    \$0x1f,%rax
 +[a-f0-9]+:	66 0f ba e8 0f       	bts    \$0xf,%ax
 +[a-f0-9]+:	48 0f ba e8 1f       	bts    \$0x1f,%rax
#pass
