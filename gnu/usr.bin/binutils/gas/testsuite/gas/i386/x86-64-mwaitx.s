# Check monitorx and mwaitx instructions

	.text
_start:
	monitorx %rax, %rcx, %rdx
	monitorx %eax, %rcx, %rdx
	monitorx
	mwaitx %rax, %rcx, %rbx
	mwaitx
