	.text
	.globl	foo
	.type	foo,%function
foo:
	.space 16
	.size foo, .-foo
	.symver foo,foo@@VERS_2.0,remove
	.globl	_Zrm1XS_
	.type	_Zrm1XS_,%function
_Zrm1XS_:
	.space 16
	.size _Zrm1XS_, .-_Zrm1XS_
	.symver _Zrm1XS_,_Zrm1XS_@@VERS_2.0,remove
