	.syntax	unified

	.text
	.align	2
	.global	_movt_abs_global
	.type	_movt_abs_global, %function
_movt_abs_global:
	bx	lr
	.size	_movt_abs_global, .-_movt_abs_global

	.global	_movw_abs_global
	.type	_movw_abs_global, %function
_movw_abs_global:
	bx	lr
	.size	_movw_abs_global, .-_movw_abs_global

	.thumb
	.align	2
	.global	_thm_movt_abs_global
	.type	_thm_movt_abs_global, %function
_thm_movt_abs_global:
	bx	lr
	.size	_thm_movt_abs_global, .-_thm_movt_abs_global

	.global	_thm_movw_abs_global
	.type	_thm_movw_abs_global, %function
_thm_movw_abs_global:
	bx	lr
	.size	_thm_movw_abs_global, .-_thm_movw_abs_global

	.global	_abs32_global_plt
	.type	_abs32_global_plt, %function
_abs32_global_plt:
	bx	lr
	.size	_abs32_global_plt, .-_abs32_global_plt

	.comm	_abs32_global,4,4
