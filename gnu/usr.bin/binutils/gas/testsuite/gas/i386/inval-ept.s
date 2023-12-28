# Check illegal EPT instructions
	.text
_start:
	invept	(%ecx), %bx
	invept	%ebx, (%ecx)
	invept	%ebx, %ecx
	invvpid	(%ecx), %bx
	invvpid %ebx, (%ecx)
	invvpid	%ebx, %ecx

	.intel_syntax noprefix
	invept bx, oword ptr [ecx]
	invept oword ptr [ecx], ebx
	invept ecx, ebx
	invvpid bx, oword ptr [ecx]
	invvpid oword ptr [ecx], ebx
	invvpid ecx, ebx
