	.cfi_sections .c6xabi.exidx
	.text
	# out of line table entry
	.global _start
	.type _start, %function
_start:
	.cfi_startproc
	.cfi_offset B3, 0
	.cfi_def_cfa_offset 8
	nop
	.p2align 6
	.cfi_endproc
	.personalityindex 3
	.handlerdata
	.word 0
	.endp

	# entry that can be merged
	.cfi_startproc
	.cfi_offset B3, 0
	.cfi_def_cfa_offset 8
	nop
	.p2align 6
	.cfi_endproc
	.personalityindex 3
	.endp

	# Section that will be placed first
	.section .before, "xa"
	.type _before, %function
_before:
	.cfi_startproc
	.cfi_offset B3, 0
	.cfi_def_cfa_offset 8
	nop
	.p2align 6
	.cfi_endproc
	.personalityindex 3
	.endp

	# section that will be placed last
	.section .after, "xa"
	.global __c6xabi_unwind_cpp_pr3
	.type __c6xabi_unwind_cpp_pr3, %function
__c6xabi_unwind_cpp_pr3:
	# entry that can be merged
	.cfi_startproc
	.cfi_offset B3, 0
	.cfi_def_cfa_offset 8
	nop
	.cfi_endproc
	.personalityindex 3
	.endp

	# final function is cantunwind, so output table size is smaller
	# than sum of input sections
	.global foo
	.type foo, %function
foo:
	.cfi_startproc
	nop
	.p2align 6
	.cfi_endproc
	.cantunwind
	.endp

	.section .far
	.word 0
