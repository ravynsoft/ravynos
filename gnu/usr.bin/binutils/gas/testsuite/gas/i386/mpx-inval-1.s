# MPX instructions
	.allow_index_reg
	.text
	.extern xxx
foo:
	bnd add %eax, %ebx  		# Bad
	bnd stosw (%edi)    		# Bad
	bnd lcall $0x1234,$xxx
	bnd ljmp $0x1234,$xxx
	bnd loop foo
	bnd jcxz foo

.intel_syntax noprefix
	bnd add ebx, eax		# Bad
	bnd stos WORD PTR[edi]		# Bad
	bnd lcall 0x1234,xxx
	bnd ljmp 0x1234,xxx
	bnd loop foo
	bnd jcxz foo
