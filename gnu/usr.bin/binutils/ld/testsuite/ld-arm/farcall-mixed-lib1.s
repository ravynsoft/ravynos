@ Create a large shared library so that calls through PLT to an undef
@ symbol require insertion of a long branch stub.
@ Check also calls to an undef weak symbol.

	.text

	.p2align 4
	.globl lib_func1
	.type lib_func1, %function
lib_func1:
	mov	ip, sp
	stmdb	sp!, {r11, ip, lr, pc}
	bl	app_func
	.weak	app_func_weak
	bl	app_func_weak
	bl	lib_func3
	bl	lib_func4
	ldmia	sp, {r11, sp, lr}
	bx lr
	.size lib_func1, . - lib_func1

	.space 0x1000000
	.p2align 4
	.globl lib_func2
	.type lib_func2, %function
	.thumb_func
	.code 16
lib_func2:
	bl	app_func
	bl	app_func_weak
	bl	lib_func3
	bl	lib_func4
	bx lr
	.size lib_func2, . - lib_func2
