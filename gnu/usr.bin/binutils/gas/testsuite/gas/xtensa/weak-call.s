	.text
	.begin	no-longcalls
	.weak	weakdef
	call8	weakdef
weakdef:
	.weak	weakref
	call8	weakref
	.end	no-longcalls
