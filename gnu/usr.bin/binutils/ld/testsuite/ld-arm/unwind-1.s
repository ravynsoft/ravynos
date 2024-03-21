	.syntax unified
	.text
	.global _start
	.type _start, %function
_start:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend

	@ Section with no unwinding information.  Linker should insert a cantunwind entry.
	.section .after, "xa"
	.global __aeabi_unwind_cpp_pr0
	.type __aeabi_unwind_cpp_pr0, %function
__aeabi_unwind_cpp_pr0:
	bx lr

	.section .far
	.word 0
