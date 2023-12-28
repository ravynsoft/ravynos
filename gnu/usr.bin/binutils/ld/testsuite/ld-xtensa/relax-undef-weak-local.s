	.type	fp,@function
	.weak	fp
	.protected fp

	.type	fh,@function
	.weak	fh
	.hidden fh

	.type	fi,@function
	.weak	fi
	.internal fi

	.globl	_start
_start:
	movi	a5, fp@plt
	movi	a5, fh@plt
	movi	a5, fi@plt

	movi	a6, fp
	movi	a6, fh
	movi	a6, fi

	.section ".text.a"
a:
	movi	a5, fp@plt
	movi	a5, fh@plt
	movi	a5, fi@plt

	movi	a6, fp
	movi	a6, fh
	movi	a6, fi
