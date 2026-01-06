

	.text
_foo:
#if __x86_64__
	.weak_reference _malloc
	.weak_reference _free
	cmpq $0, _malloc@GOTPCREL(%rip)
	cmpq $0xFFFF, _free@GOTPCREL(%rip)
#endif
	nop
	
