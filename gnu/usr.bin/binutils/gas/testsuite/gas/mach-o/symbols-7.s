
L01:	.space 10

L02:	.space 10


	.non_lazy_symbol_pointer
	
	a = 5
	.indirect_symbol a
	.space 4
	
	.indirect_symbol L01
	.long L01-.
	
	.indirect_symbol b
	.space 4
	
	b = 10
	
	.globl c
	c = 20
	.indirect_symbol c
	.space 4
