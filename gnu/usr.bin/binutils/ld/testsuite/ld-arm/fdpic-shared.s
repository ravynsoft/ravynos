	.arch armv7-r
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"shared.c"
	.text
	.align	2
	.global	my_shared_func1
	.syntax unified
	.arm
	.fpu softvfp
	.type	my_shared_func1, %function
my_shared_func1:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.size	my_shared_func1, .-my_shared_func1
	.align	2
	.global	my_shared_func3
	.syntax unified
	.arm
	.fpu softvfp
	.type	my_shared_func3, %function
my_shared_func3:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r0, #0
	bx	lr
	.size	my_shared_func3, .-my_shared_func3
	.align	2
	.global	my_shared_func2
	.syntax unified
	.arm
	.fpu softvfp
	.type	my_shared_func2, %function
my_shared_func2:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	mov	r4, r9
	bl	my_shared_func3(PLT)
	mov	r9, r4
	pop	{r4, pc}
	.size	my_shared_func2, .-my_shared_func2
