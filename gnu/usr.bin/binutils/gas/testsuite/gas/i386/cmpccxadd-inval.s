# Check Illegal CMPccXADD instructions

	.allow_index_reg
	.text
_start:
	cmpbexadd	%eax, %eax, 0x10000000(%esp, %esi, 8)
	cmpbxadd	%ebx, %ebx, (%ecx)
	cmplexadd	%eax, %eax, 508(%ecx)
	cmplxadd	%ebx, %ebx, -512(%edx)
