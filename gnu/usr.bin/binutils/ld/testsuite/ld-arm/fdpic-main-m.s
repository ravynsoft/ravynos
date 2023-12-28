	.arch armv7-m
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"hello.c"
	.text
	.align	2
	.syntax unified
	.thumb
	.fpu softvfp
	.type	my_local_func, %function
my_local_func:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.size	my_local_func, .-my_local_func
	.section	.text.startup,"ax",%progbits
	.align	2
	.global	_start
	.syntax unified
	.thumb
	.fpu softvfp
	.type	_start, %function
_start:
	b	main

	.global	main
	.syntax unified
	.thumb
	.fpu softvfp
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r2, .L4
	ldr	r3, .L4+4
	push	{r4, r5, r6, lr}
	ldr	r5, [r9, r2]
	mov	r4, r9
	ldr	r3, [r9, r3]
	str	r3, [r5]
	bl	my_shared_func1(PLT)
	mov	r6, r0
	mov	r9, r4
	ldr	r0, [r5]
	mov	r9, r4
	bl	my_shared_func2(PLT)
	ldr	r3, .L4+8
	mov	r9, r4
	add	r3, r3, r9
	mov	r0, r3
	str	r3, [r5]
	mov	r9, r4
	bl	my_shared_func2(PLT)
	ldr	r3, .L4+12
	mov	r9, r4
	ldr	r3, [r9, r3]
	ldr	r0, [r3]
	mov	r9, r4
	bl	my_shared_func2(PLT)
	mov	r0, r6
	mov	r9, r4
	pop	{r4, r5, r6, pc}
.L5:
	.align	2
.L4:
	.word	funcptr(GOT)
	.word	my_shared_func1(GOTFUNCDESC)
	.word	my_local_func(GOTOFFFUNCDESC)
	.word	funcptr2(GOT)
	.size	main, .-main
	.global	funcptr2
	.comm	funcptr,4,4
	.section	.data.rel,"aw",%progbits
	.align	2
	.type	funcptr2, %object
	.size	funcptr2, 4
funcptr2:
	.word	my_shared_func1(FUNCDESC)
