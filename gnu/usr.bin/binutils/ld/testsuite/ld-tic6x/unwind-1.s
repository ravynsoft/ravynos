	.cfi_sections .c6xabi.exidx
	.text
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
	.endp

	# Section with no unwinding information.
	# Linker should insert a cantunwind entry.
	.section .after, "xa"
	.global __c6xabi_unwind_cpp_pr3
	.type __c6xabi_unwind_cpp_pr3, %function
__c6xabi_unwind_cpp_pr3:
	nop
	.p2align 6

	.section .far
	.word 0
