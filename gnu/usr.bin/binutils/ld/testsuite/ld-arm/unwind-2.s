	.syntax unified
	.text

	.global __aeabi_unwind_cpp_pr0
	.type __aeabi_unwind_cpp_pr0, %function
__aeabi_unwind_cpp_pr0:
	.global _start
	.type _start, %function
_start:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend

	@ last text section has unwind information. Linker should append a
	@ terminating cantunwind entry.

	.section .far
	.word 0
