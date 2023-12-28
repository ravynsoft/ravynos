	.text
	.p2align 4,,15
foo:
	shrl	$2, %ecx
.L1:
	shrl	$2, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	xorl	%eax, %eax
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	testb	$2, %dl
	rep stosl
	jne	.L1
	xorl	%eax, %eax
	shrl	$2, %ecx
.L2:
	shrl	$2, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	xorl	%eax, %eax
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	testb	$2, %dl
	call	bar
	jne	.L2
	xorl	%eax, %eax
