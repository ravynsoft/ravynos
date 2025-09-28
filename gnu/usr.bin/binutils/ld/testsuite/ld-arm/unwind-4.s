	.syntax unified
	.text
	@ out of line table entry
	.global _start
	.type _start, %function
_start:
	.fnstart
	.save {r4, lr}
	.vsave {d0}
	.vsave {d4}
	bl _before
	.fnend

	@ entry that can be merged
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend

	@ Section that will be placed first
	.section .before, "xa"
	.type _before, %function
_before:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend

	@ section that will be placed last
	.section .after, "xa"
	.global __aeabi_unwind_cpp_pr0
	.type __aeabi_unwind_cpp_pr0, %function
__aeabi_unwind_cpp_pr0:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend
	@ final function is cantunwind, so output table size is smaller
	@ than sum of input sections
	.global __aeabi_unwind_cpp_pr1
	.type __aeabi_unwind_cpp_pr1, %function
__aeabi_unwind_cpp_pr1:
	.fnstart
	.cantunwind
	bx lr
	.fnend

	.section .far
	.word 0
