# 64-bit only invalid MOVSXD with AMD64 ISA
	.text
_start:
	movslq	%ax, %cx
	movslq	%eax, %ecx
	movslq	(%rax), %ecx
	movsxd	%ax, %cx

	.intel_syntax noprefix
	movslq	cx, eax
	movslq	ecx, eax
	movslq	ecx, [rax]
	movsxd	cx, ax
	movsxd	cx, WORD PTR [rax]
