	.syntax unified
	.text
	@ section without unwind info
	.global _start
	.type _start, %function
_start:
	bl _before

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

	.section .far
	.word 0
