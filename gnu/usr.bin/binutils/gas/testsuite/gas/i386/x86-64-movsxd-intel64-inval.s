# 64-bit only invalid MOVSXD with Intel64 ISA
	.text
_start:
	movslq	%eax, %cx
	movslq	%eax, %ecx
	movslq	(%rax), %ecx
	movsxd	%ax, %ecx

	.intel_syntax noprefix
	movslq	cx, ax
	movslq	ecx, eax
	movslq	ecx, [rax]
	movsxd	cx, eax
	movsxd	cx, DWORD PTR [rax]
