	.text
	.p2align 4
	.globl _start
_start:
	mov	ip, sp
	stmdb	sp!, {r11, ip, lr, pc}
	bl	app_func
	bl	lib_func1
	bl	lib_func2
	ldmia	sp, {r11, sp, lr}
	bx lr

	.p2align 4
	.globl app_tfunc_close
	.type app_tfunc_close,%function
	.thumb_func
	.code 16
app_tfunc_close:
	push	{lr}
	bl	lib_func2
	pop	{pc}
	bx	lr

@ We will place the section .far_arm at 0x2100000.
	.section .far_arm, "xa"

	.arm
	.p2align 4
	.globl app_func
	.type app_func,%function
app_func:
	mov	ip, sp
	stmdb	sp!, {r11, ip, lr, pc}
	bl	lib_func1
	bl	lib_func2
	ldmia	sp, {r11, sp, lr}
	bx lr

	.arm
	.p2align 4
	.globl app_func2
	.type app_func2,%function
app_func2:
	bx	lr

@ We will place the section .far_thumb at 0x2200000.
	.section .far_thumb, "xa"

	.p2align 4
	.globl app_tfunc
	.type app_tfunc,%function
	.thumb_func
	.code 16
app_tfunc:
	push	{lr}
	bl	lib_func2
	pop	{pc}
	bx	lr

	.data
	.long data_obj
