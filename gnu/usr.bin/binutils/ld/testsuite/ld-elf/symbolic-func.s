	.text
	.global fun
	.type	fun, %function
fun:
	.space	4
	.size	fun, .-fun

	.section .data.rel.ro,"aw",%progbits
	.p2align 3
	.type	fun_ptr, %object
fun_ptr:
	.dc.a	fun
	.size	fun_ptr, .-fun_ptr
