	.arch armv8-a
	.text
	.align	2
	.global	_Z5foo_av
	.type	_Z5foo_av, %function
_Z5foo_av:
.LFB0:
	.cfi_startproc
	hint	25 // paciasp
	.cfi_negate_ra_state
	stp	x29, x30, [sp, -16]!
	.cfi_def_cfa_offset 16
	.cfi_offset 29, -16
	.cfi_offset 30, -8
	.cfi_endproc
.LFE0:
	.size	_Z5foo_av, .-_Z5foo_av
	.align	2
	.global	_Z5foo_bv
	.type	_Z5foo_bv, %function
