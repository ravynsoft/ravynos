	.text
_start:
	vldmxcsr (%ebp)
	lgdt (%ebp)
	vmptrld (%ebp)
	vmclear (%ebp)
	invpcid (%ebp), %edx
	invlpg (%ebp)
	clflush (%ebp)
	clflushopt (%ebp)
	clwb (%ebp)
	cldemote (%ebp)
	bndmk (%ebp), %bnd1
	bndcl (%ebp), %bnd1
	bndcu (%ebp), %bnd1
	bndcn (%ebp), %bnd1
	bndstx %bnd1, (%ebp)
	bndldx (%ebp), %bnd1
	prefetcht0 (%ebp)
	prefetcht1 (%ebp)
	prefetcht2 (%ebp)
	prefetchw (%ebp)
	pop %ds
	popf
	popa
	xlatb (%ebx)
	fsts (%ebp)
	flds (%ebp)
	fistl (%ebp)
	fists (%ebp)
	fildl (%ebp)
	filds (%ebp)
	fsave (%ebp)
	frstor (%ebp)
	filds (%ebp)
	fisttps (%ebp)
	fldenv (%ebp)
	fstenv (%ebp)
	fadds  (%ebp)
	fadds  (%esp)
	fadd  %st(3),%st
	fadds (%ecx)
	filds (%ecx)
	fists (%ecx)
	xrstor (%ecx)
	prefetchnta (%ecx)
	cmpxchg8b (%ecx)
	incl	%ecx
	lgdt (%eax)
	pfcmpeq 2(%esi),%mm4
	popl (%eax)
	popl %eax
	rclw	(%ecx)
	testl	$1,(%ecx)
	incl	(%ecx)
	notl	(%ecx)
	divl	(%ecx)
	mull	(%ecx)
	idivl	(%ecx)
	imull	(%ecx)
	leal	(%eax,%eax,2), %eax
	leave
	outsb
	lodsb
	rep movsl
	rep scasl
	rep cmpsl
	rep lodsl
	addl $1, (%eax)
	btl $1, (%eax)
	xadd %eax,(%ebx)
	xadd %eax,%ebx
	xchg %eax,(%ebx)
	xchg %eax,%ebx
	cmp    %eax,0x40(%ebp)
	cmp    0x40(%ebp),%eax
	add %eax,0x40(%ebp)
	add (%eax),%eax
	test %eax,0x40(%ebp)
	test 0x40(%ebp),%eax
