	.data
	.global __expobj2
	.type	__expobj2,@object
	.size	__expobj2,4
__expobj2:
	.dword 0
	.weak	expobj2
	.set	expobj2,__expobj2

	.text
	.global _start
_start:
	nop
	.global __expfn2
__expfn2:
	.type	__expfn2,@function
	nop
.Lfe1:
	.size	__expfn2,.Lfe1-__expfn2
	.weak	expfn2
	.set	expfn2,__expfn2
