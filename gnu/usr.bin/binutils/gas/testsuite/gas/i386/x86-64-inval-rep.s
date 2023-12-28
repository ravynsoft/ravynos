 	.text
	rep add %rbx, %rax
	repe add %rbx, %rax
	repz add %rbx, %rax
	repne add %rbx, %rax
	repnz add %rbx, %rax
