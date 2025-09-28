# Check illegal 64bit INVPCID instructions
	.text
_start:
	invvpid	(%rcx), %bx
	invvpid	(%rcx), %ebx
	invvpid %rbx, (%rcx)
	invvpid	%rbx, %rcx

	.intel_syntax noprefix
	invvpid bx, [rcx]
	invvpid ebx, [rcx]
	invvpid [rcx], rbx
	invvpid rcx, rbx
