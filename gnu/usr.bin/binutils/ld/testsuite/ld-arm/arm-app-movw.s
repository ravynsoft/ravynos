	.text
	.globl _start
_start:
	movw	r0, #:lower16:data_obj
	movt	r0, #:upper16:data_obj
	movw	r0, #:lower16:lib_func1
	movt	r0, #:upper16:lib_func1

	.globl app_func2
app_func2:
	bx	lr
