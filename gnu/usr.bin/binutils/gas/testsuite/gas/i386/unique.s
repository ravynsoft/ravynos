	.section .text,"ax",@progbits,unique,1
foo:
	mov %eax, %ebx
	.section .text,"ax",@progbits,unique,2
bar:
	xor %eax, %ebx
	.section .text,"ax",@progbits,unique,1
	ret
	.section .text,"ax",@progbits,unique,2
	ret
	.section .text,"axG",@progbits,foo,comdat,unique,1
foo1:
	mov	%eax, %ebx
	.section .text,"axG",@progbits,bar,comdat,unique,1
bar1:
	add	%eax, %ebx
	.section .text,"axG",@progbits,bar,comdat,unique,2
bar2:
	sub	%eax, %ebx
	.section .text,"axG",@progbits,foo,comdat,unique,2
foo2:
	xor	%eax, %ebx
	.section .text,"axG",@progbits,bar,comdat,unique,1
	nop
	ret
	.section .text,"axG",@progbits,foo,comdat,unique,1
	ret
	.section .text,"axG",@progbits,bar,comdat,unique,2
	nop
	nop
	nop
	ret
	.section .text,"axG",@progbits,foo,comdat,unique,2
	nop
	nop
	ret
