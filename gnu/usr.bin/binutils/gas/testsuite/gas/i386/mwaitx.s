# Check monitorx and mwaitx instructions

	.text
_start:
	monitorx %eax, %ecx, %edx
	monitorx %ax, %ecx, %edx
	monitorx
	mwaitx %eax, %ecx, %ebx
	mwaitx
