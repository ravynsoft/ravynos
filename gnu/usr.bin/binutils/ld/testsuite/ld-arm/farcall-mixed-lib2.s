@ Create a large shared library so that calls through PLT to an undef
@ symbol require insertion of a long branch stub.
@ Check also calls to an undef weak symbol.

	.text

	.space 0x1000000
	.p2align 4
	.globl lib_func3
	.type lib_func3, %function
	.thumb_func
	.code 16
lib_func3:
	bl	app_func
	.weak	app_func_weak
	bl	app_func_weak
	bx lr
	.size lib_func3, . - lib_func3
