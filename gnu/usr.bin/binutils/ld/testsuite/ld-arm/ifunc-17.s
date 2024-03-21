	.syntax unified
	.arch armv6t2

	.global appfunc1
	.type	appfunc1,%gnu_indirect_function
	.thumb
appfunc1:
	mov	pc,lr
	.size	appfunc1,.-appfunc1

	.global appfunc2
	.type	appfunc2,%gnu_indirect_function
	.thumb
appfunc2:
	mov	pc,lr
	.size	appfunc2,.-appfunc2

	.global _start
	.type _start,%function
	.thumb
_start:
	bl	appfunc1(PLT)
	.word	appfunc2(GOT)
	.size	_start,.-_start
