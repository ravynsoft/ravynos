#as: -mlfence-after-load=yes
#objdump: -dw
#warning_output: lfence-load.e
#name: -mlfence-after-load=yes

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr 0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 01 55 00          	lgdtl  0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f c7 75 00          	vmptrld 0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 0f c7 75 00       	vmclear 0x0\(%ebp\)
 +[a-f0-9]+:	66 0f 38 82 55 00    	invpcid 0x0\(%ebp\),%edx
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 01 7d 00          	invlpg 0x0\(%ebp\)
 +[a-f0-9]+:	0f ae 7d 00          	clflush 0x0\(%ebp\)
 +[a-f0-9]+:	66 0f ae 7d 00       	clflushopt 0x0\(%ebp\)
 +[a-f0-9]+:	66 0f ae 75 00       	clwb   0x0\(%ebp\)
 +[a-f0-9]+:	0f 1c 45 00          	cldemote 0x0\(%ebp\)
 +[a-f0-9]+:	f3 0f 1b 4d 00       	bndmk  0x0\(%ebp\),%bnd1
 +[a-f0-9]+:	f3 0f 1a 4d 00       	bndcl  0x0\(%ebp\),%bnd1
 +[a-f0-9]+:	f2 0f 1a 4d 00       	bndcu  0x0\(%ebp\),%bnd1
 +[a-f0-9]+:	f2 0f 1b 4d 00       	bndcn  0x0\(%ebp\),%bnd1
 +[a-f0-9]+:	0f 1b 4d 00          	bndstx %bnd1,0x0\(%ebp\)
 +[a-f0-9]+:	0f 1a 4d 00          	bndldx 0x0\(%ebp\),%bnd1
 +[a-f0-9]+:	0f 18 4d 00          	prefetcht0 0x0\(%ebp\)
 +[a-f0-9]+:	0f 18 55 00          	prefetcht1 0x0\(%ebp\)
 +[a-f0-9]+:	0f 18 5d 00          	prefetcht2 0x0\(%ebp\)
 +[a-f0-9]+:	0f 0d 4d 00          	prefetchw 0x0\(%ebp\)
 +[a-f0-9]+:	1f                   	pop    %ds
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9d                   	popf
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	61                   	popa
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d7                   	xlat   %ds:\(%ebx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d9 55 00             	fsts   0x0\(%ebp\)
 +[a-f0-9]+:	d9 45 00             	flds   0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	db 55 00             	fistl  0x0\(%ebp\)
 +[a-f0-9]+:	df 55 00             	fists  0x0\(%ebp\)
 +[a-f0-9]+:	db 45 00             	fildl  0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 45 00             	filds  0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9b dd 75 00          	fsave  0x0\(%ebp\)
 +[a-f0-9]+:	dd 65 00             	frstor 0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 45 00             	filds  0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 4d 00             	fisttps 0x0\(%ebp\)
 +[a-f0-9]+:	d9 65 00             	fldenv 0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	9b d9 75 00          	fstenv 0x0\(%ebp\)
 +[a-f0-9]+:	d8 45 00             	fadds  0x0\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d8 04 24             	fadds  \(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	d8 c3                	fadd   %st\(3\),%st
 +[a-f0-9]+:	d8 01                	fadds  \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 01                	filds  \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	df 11                	fists  \(%ecx\)
 +[a-f0-9]+:	0f ae 29             	xrstor \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 18 01             	prefetchnta \(%ecx\)
 +[a-f0-9]+:	0f c7 09             	cmpxchg8b \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	41                   	inc    %ecx
 +[a-f0-9]+:	0f 01 10             	lgdtl  \(%eax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f 0f 66 02 b0       	pfcmpeq 0x2\(%esi\),%mm4
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	8f 00                	pop    \(%eax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	58                   	pop    %eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 d1 11             	rclw   \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 01 01 00 00 00    	testl  \$0x1,\(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff 01                	incl   \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 11                	notl   \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 31                	divl   \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 21                	mull   \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 39                	idivl  \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f7 29                	imull  \(%ecx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	8d 04 40             	lea    \(%eax,%eax,2\),%eax
 +[a-f0-9]+:	c9                   	leave
 +[a-f0-9]+:	6e                   	outsb  %ds:\(%esi\),\(%dx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ac                   	lods   %ds:\(%esi\),%al
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 a5                	rep movsl %ds:\(%esi\),%es:\(%edi\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 af                	repz scas %es:\(%edi\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 a7                	repz cmpsl %es:\(%edi\),%ds:\(%esi\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 ad                	rep lods %ds:\(%esi\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	83 00 01             	addl   \$0x1,\(%eax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f ba 20 01          	btl    \$0x1,\(%eax\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f c1 03             	xadd   %eax,\(%ebx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	0f c1 c3             	xadd   %eax,%ebx
 +[a-f0-9]+:	87 03                	xchg   %eax,\(%ebx\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	93                   	xchg   %eax,%ebx
 +[a-f0-9]+:	39 45 40             	cmp    %eax,0x40\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	3b 45 40             	cmp    0x40\(%ebp\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	01 45 40             	add    %eax,0x40\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	03 00                	add    \(%eax\),%eax
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	85 45 40             	test   %eax,0x40\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	85 45 40             	test   %eax,0x40\(%ebp\)
 +[a-f0-9]+:	0f ae e8             	lfence
#pass
