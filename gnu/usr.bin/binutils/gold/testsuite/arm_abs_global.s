	.syntax	unified

	.text
	.align	2

	.global	_start
	.type	_start, %function
_start:
	bx	lr
	.size	_start, .-_start

	.global	_arm_test
	.type	_arm_test, %function
_arm_test:
	movt	r0, #:upper16:_movt_abs_global
	movw	r0, #:lower16:_movw_abs_global
	bx	lr
	.size	_arm_test, .-_arm_test

	.thumb
	.global	_thumb_test
_thumb_test:
	movt	r0, #:upper16:_thm_movt_abs_global
	movw	r0, #:lower16:_thm_movw_abs_global
	bx	lr
	.size	_thumb_test, .-_thumb_test

	.data
_data_test:
	.word	_abs32_global
	.word	_abs32_global_plt
