# MPX instructions
	.allow_index_reg
	.text
	bnd add %rax, %rbx  		# Bad
	bnd stosw (%edi)    		# Bad
	bnd loop foo
	bnd jrcxz foo

.intel_syntax noprefix
	bnd add rbx, rax		# Bad
	bnd stos WORD PTR [edi]		# Bad
	bnd loop foo
	bnd jrcxz foo
