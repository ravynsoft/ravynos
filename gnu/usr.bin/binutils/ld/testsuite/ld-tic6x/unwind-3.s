	.cfi_sections .c6xabi.exidx
	.text
	# section without unwind info
	.global _start
	.type _start, %function
_start:
	b .s2 _before
	nop 5
	.p2align 6

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
	.cfi_startproc
	.cfi_offset B10, 0
	.cfi_def_cfa_offset 8
	nop
	.p2align 6
	.cfi_endproc
	.personalityindex 3
	.endp

	.section .far
	.word 0
