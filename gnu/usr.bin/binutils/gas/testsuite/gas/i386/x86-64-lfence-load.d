#as: -mlfence-after-load=yes
#objdump: -dw
#warning_output: lfence-load.e
#name: x86-64 -mlfence-after-load=yes

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr 0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 01 55 00          	lgdt   0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f c7 75 00          	vmptrld 0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 0f c7 75 00       	vmclear 0x0\(%rbp\)
 +[a-f0-9]+:	66 0f 38 82 55 00    	invpcid 0x0\(%rbp\),%rdx
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	67 0f 01 38          	invlpg \(%eax\)
 +[a-f0-9]+:	0f ae 7d 00          	clflush 0x0\(%rbp\)
 +[a-f0-9]+:	66 0f ae 7d 00       	clflushopt 0x0\(%rbp\)
 +[a-f0-9]+:	66 0f ae 75 00       	clwb   0x0\(%rbp\)
 +[a-f0-9]+:	0f 1c 45 00          	cldemote 0x0\(%rbp\)
 +[a-f0-9]+:	f3 0f 1b 4d 00       	bndmk  0x0\(%rbp\),%bnd1
 +[a-f0-9]+:	f3 0f 1a 4d 00       	bndcl  0x0\(%rbp\),%bnd1
 +[a-f0-9]+:	f2 0f 1a 4d 00       	bndcu  0x0\(%rbp\),%bnd1
 +[a-f0-9]+:	f2 0f 1b 4d 00       	bndcn  0x0\(%rbp\),%bnd1
 +[a-f0-9]+:	0f 1b 4d 00          	bndstx %bnd1,0x0\(%rbp\)
 +[a-f0-9]+:	0f 1a 4d 00          	bndldx 0x0\(%rbp\),%bnd1
 +[a-f0-9]+:	0f 18 4d 00          	prefetcht0 0x0\(%rbp\)
 +[a-f0-9]+:	0f 18 55 00          	prefetcht1 0x0\(%rbp\)
 +[a-f0-9]+:	0f 18 5d 00          	prefetcht2 0x0\(%rbp\)
 +[a-f0-9]+:	0f 0d 4d 00          	prefetchw 0x0\(%rbp\)
 +[a-f0-9]+:	0f 18 3d 78 56 34 12 	prefetchit0 0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
 +[a-f0-9]+:	0f 18 35 78 56 34 12 	prefetchit1 0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
 +[a-f0-9]+:	0f a1                	pop    %fs
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9d                   	popf
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d7                   	xlat   %ds:\(%rbx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d9 55 00             	fsts   0x0\(%rbp\)
 +[a-f0-9]+:	d9 45 00             	flds   0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	db 55 00             	fistl  0x0\(%rbp\)
 +[a-f0-9]+:	df 55 00             	fists  0x0\(%rbp\)
 +[a-f0-9]+:	db 5d 00             	fistpl 0x0\(%rbp\)
 +[a-f0-9]+:	df 5d 00             	fistps 0x0\(%rbp\)
 +[a-f0-9]+:	df 7d 00             	fistpll 0x0\(%rbp\)
 +[a-f0-9]+:	db 45 00             	fildl  0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 45 00             	filds  0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 6d 00             	fildll 0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9b dd 75 00          	fsave  0x0\(%rbp\)
 +[a-f0-9]+:	dd 65 00             	frstor 0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	db 4d 00             	fisttpl 0x0\(%rbp\)
 +[a-f0-9]+:	df 4d 00             	fisttps 0x0\(%rbp\)
 +[a-f0-9]+:	dd 4d 00             	fisttpll 0x0\(%rbp\)
 +[a-f0-9]+:	d9 65 00             	fldenv 0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9b d9 75 00          	fstenv 0x0\(%rbp\)
 +[a-f0-9]+:	d8 45 00             	fadds  0x0\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d8 04 24             	fadds  \(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d8 c3                	fadd   %st\(3\),%st
 +[a-f0-9]+:	d8 01                	fadds  \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 01                	filds  \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 11                	fists  \(%rcx\)
 +[a-f0-9]+:	0f ae 29             	xrstor \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 18 01             	prefetchnta \(%rcx\)
 +[a-f0-9]+:	0f c7 09             	cmpxchg8b \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 0f c7 09          	cmpxchg16b \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff c1                	inc    %ecx
 +[a-f0-9]+:	0f 01 10             	lgdt   \(%rax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 0f 66 02 b0       	pfcmpeq 0x2\(%rsi\),%mm4
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	8f 00                	pop    \(%rax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	58                   	pop    %rax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 d1 11             	rclw   \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 01 01 00 00 00    	testl  \$0x1,\(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff 01                	incl   \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 11                	notl   \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 31                	divl   \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 21                	mull   \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 39                	idivl  \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 29                	imull  \(%rcx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 8d 04 40          	lea    \(%rax,%rax,2\),%rax
 +[a-f0-9]+:	c9                   	leave
 +[a-f0-9]+:	6e                   	outsb  %ds:\(%rsi\),\(%dx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ac                   	lods   %ds:\(%rsi\),%al
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 a5                	rep movsl %ds:\(%rsi\),%es:\(%rdi\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 af                	repz scas %es:\(%rdi\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 a7                	repz cmpsl %es:\(%rdi\),%ds:\(%rsi\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 ad                	rep lods %ds:\(%rsi\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	41 83 03 01          	addl   \$0x1,\(%r11\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	41 0f ba 23 01       	btl    \$0x1,\(%r11\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 0f c1 03          	xadd   %rax,\(%rbx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 0f c1 c3          	xadd   %rax,%rbx
 +[a-f0-9]+:	48 87 03             	xchg   %rax,\(%rbx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 93                	xchg   %rax,%rbx
 +[a-f0-9]+:	48 39 45 40          	cmp    %rax,0x40\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 3b 45 40          	cmp    0x40\(%rbp\),%rax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 01 45 40          	add    %rax,0x40\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 03 00             	add    \(%rax\),%rax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 85 45 40          	test   %rax,0x40\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	48 85 45 40          	test   %rax,0x40\(%rbp\)
 +[a-f0-9]+:	0f ae e8             	lfence
#pass
