# Check 64bit EPT instructions
	.text
_start:
	invept	(%rcx), %rbx
	invept	(%rcx), %r11
	invvpid	(%rcx), %rbx
	invvpid	(%rcx), %r11

	.intel_syntax noprefix
	invept rbx, oword ptr [rcx]
	invept r11, oword ptr [rcx]
	invvpid rbx, oword ptr [rcx]
	invvpid r11, oword ptr [rcx]
