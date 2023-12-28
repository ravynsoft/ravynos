# Check illegal EPT instructions in 64bit mode.
	.text
_start:
	invept	(%rcx), %bx
	invept	(%rcx), %ebx
	invept	%rbx, (%rcx)
	invept	%rbx, %rcx
	invvpid	(%rcx), %bx
	invvpid	(%rcx), %ebx
	invvpid %rbx, (%rcx)
	invvpid	%rbx, %rcx

	.intel_syntax noprefix
	invept bx, oword ptr [rcx]
	invept ebx, oword ptr [rcx]
	invept oword ptr [rcx], rbx
	invept rcx, rbx
	invvpid bx, oword ptr [rcx]
	invvpid ebx, oword ptr [rcx]
	invvpid oword ptr [rcx], rbx
	invvpid rcx, rbx
