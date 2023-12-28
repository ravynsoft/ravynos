 	.text
	rep add %ebx, %eax
	repe add %ebx, %eax
	repz add %ebx, %eax
	repne add %ebx, %eax
	repnz add %ebx, %eax
