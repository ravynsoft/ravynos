# 64-bit only MOVSXD with Intel64 ISA
	.text
_start:
	movslq	%eax, %rcx
	movslq	(%rax), %rcx
	movsxd	%eax, %ecx
	movsxd	(%rax), %ecx
	movsxd	%ax, %cx
	movsxd	(%rax), %cx

	.intel_syntax noprefix
	movsxd	rcx, eax
	movsxd	rcx, DWORD PTR [rax]
	movsxd	rcx, [rax]
	movsxd	ecx, eax
	movsxd	ecx, DWORD PTR [rax]
	movsxd	ecx, [rax]
	movsxd	cx, ax
	movsxd	cx, WORD PTR [rax]
	movsxd	cx, [rax]
