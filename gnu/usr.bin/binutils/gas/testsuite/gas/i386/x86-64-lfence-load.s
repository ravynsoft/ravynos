	.text
_start:
	vldmxcsr (%rbp)
	lgdt (%rbp)
	vmptrld (%rbp)
	vmclear (%rbp)
	invpcid (%rbp), %rdx
	invlpg (%eax)
	clflush (%rbp)
	clflushopt (%rbp)
	clwb (%rbp)
	cldemote (%rbp)
	bndmk (%rbp), %bnd1
	bndcl (%rbp), %bnd1
	bndcu (%rbp), %bnd1
	bndcn (%rbp), %bnd1
	bndstx %bnd1, (%rbp)
	bndldx (%rbp), %bnd1
	prefetcht0 (%rbp)
	prefetcht1 (%rbp)
	prefetcht2 (%rbp)
	prefetchw (%rbp)
	prefetchit0 0x12345678(%rip)
	prefetchit1 0x12345678(%rip)
	pop %fs
	popf
	xlatb (%rbx)
	fsts (%rbp)
	flds (%rbp)
	fistl (%rbp)
	fists (%rbp)
	fistpl (%rbp)
	fistps (%rbp)
	fistpq (%rbp)
	fildl (%rbp)
	filds (%rbp)
	fildq (%rbp)
	fsave (%rbp)
	frstor (%rbp)
	fisttpl (%rbp)
	fisttps (%rbp)
	fisttpq (%rbp)
	fldenv (%rbp)
	fstenv (%rbp)
	fadds  (%rbp)
	fadds  (%rsp)
	fadd  %st(3),%st
	fadds (%rcx)
	filds (%rcx)
	fists (%rcx)
	xrstor (%rcx)
	prefetchnta (%rcx)
	cmpxchg8b (%rcx)
	cmpxchg16b (%rcx)
	incl	%ecx
	lgdt (%rax)
	pfcmpeq 2(%rsi),%mm4
	popq (%rax)
	popq %rax
	rclw	(%rcx)
	testl	$1,(%rcx)
	incl	(%rcx)
	notl	(%rcx)
	divl	(%rcx)
	mull	(%rcx)
	idivl	(%rcx)
	imull	(%rcx)
	leaq	(%rax,%rax,2), %rax
	leave
	outsb
	lodsb
	rep movsl
	rep scasl
	rep cmpsl
	rep lodsl
	addl $1, (%r11)
	btl $1, (%r11)
	xadd %rax,(%rbx)
	xadd %rax,%rbx
	xchg %rax,(%rbx)
	xchg %rax,%rbx
	cmp    %rax,0x40(%rbp)
	cmp    0x40(%rbp),%rax
	add %rax,0x40(%rbp)
	add (%rax),%rax
	test %rax,0x40(%rbp)
	test 0x40(%rbp),%rax
