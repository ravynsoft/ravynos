	.text
	.p2align 4,,15
foo:
	shrl	$2, %ecx
l1:
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
	cmpb	$2, %dl
	jo	l1
	xorl	%eax, %eax
	shrl	$2, %ecx
l2:
	shrl	$2, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	xorl	%eax, %eax
	shrl	$2, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	testb	$2, %dl
	jne	l2
	xorl	%eax, %eax
l3:
	shrl	$2, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	shrl	$2, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	shrl	$2, %ecx
	movl	%edx, %ecx
	xorl	%eax, %eax
	inc	%eax
	jbe	l2
	xorl	%eax, %eax
