	.type	fd,@function
	.weak	fd

	.globl	_start
_start:
	movi	a5, fd@plt
	movi	a6, fd

	.section ".text.a"
a:
	movi	a5, fd@plt
	movi	a6, fd
