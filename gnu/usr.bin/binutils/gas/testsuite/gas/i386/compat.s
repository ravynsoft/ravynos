# Check SYSV mnemonic instructions.
	.text 
	fsub	%st,%st(3)
	fsub	%st(3),%st
	fsubp
	fsubp	%st(3)
	fsubp	%st,%st(3)
	fsubr	%st,%st(3)
	fsubr	%st(3),%st
	fsubrp
	fsubrp	%st(3)
	fsubrp	%st,%st(3)
	fdiv	%st,%st(3)
	fdiv	%st(3),%st
	fdivp
	fdivp	%st(3)
	fdivp	%st,%st(3)
	fdivr	%st,%st(3)
	fdivr	%st(3),%st
	fdivrp
	fdivrp	%st(3)
	fdivrp	%st,%st(3)
