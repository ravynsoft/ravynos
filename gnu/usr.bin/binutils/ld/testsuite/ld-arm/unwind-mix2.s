	.syntax unified
	.text
	.global __aeabi_unwind_cpp_pr1
	.type __aeabi_unwind_cpp_pr1, %function
__aeabi_unwind_cpp_pr1:
	bx lr

	.global end
	.type end, %function
end:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend

