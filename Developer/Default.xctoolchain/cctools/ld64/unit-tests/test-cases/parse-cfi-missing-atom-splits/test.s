	.section	__TEXT,__text,regular,pure_instructions
	.p2align 2

	.global foo
foo:
	.cfi_startproc
	.cfi_personality 155, ___gxx_personality_v0
	.cfi_lsda 16, LLSDAFOO
	ret
	.cfi_endproc

bar:
	.cfi_startproc
	.cfi_lsda 16, LLSDABAR
	ret
	.cfi_endproc

	# A bug introduced in
	# rdar://93130909 (Tweak parsing and stabs generation of empty atoms)
	# caused any remaining CFI start addresses to be ignored.
	# That resulted in parsing `__func_terminate` as a single large atom, instead of
	# splitting it at the `L__func_terminate_two` label.
	.section	__TEXT,__text_alt,regular,pure_instructions
	.p2align 2
__func_terminate:
	.cfi_startproc
	stp	x29, x30, [sp, #-16]!
	.cfi_def_cfa_offset 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ret
	.cfi_endproc

L__func_terminate_two:
	.cfi_startproc
	stp	x29, x30, [sp, #-16]!
	.cfi_def_cfa_offset 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ret
	.cfi_endproc

	.section	__TEXT,__cstring,cstring_literals
l_.str:
	.asciz	"bar"

	.section	__TEXT,__gcc_except_tab
	.p2align	3
GCC_except_table0:
LLSDAFOO:
	.byte	0
	.byte	0

GCC_except_table1:
LLSDABAR:
	.byte	0
	.byte	0

.subsections_via_symbols
