@ Test to ensure that ARM calls exceeding 32Mb generate stubs,
@ and that a large input section forces stub insertion before its
@ contribution.

	.text
myfunc:
	.space 0x2000000
	bl bar

	.section .far, "xa"
	.type bar, %function
	.global bar
bar:
	bx lr
