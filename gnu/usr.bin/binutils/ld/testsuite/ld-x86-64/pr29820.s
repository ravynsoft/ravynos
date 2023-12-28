	.text
	leaq _TLS_MODULE_BASE_@tlsdesc(%rip), %rax
	call *_TLS_MODULE_BASE_@tlscall(%rax)
	movl %fs:a@dtpoff(%rax), %edx
	addl %fs:b@dtpoff(%rax), %edx

	.section .tbss
	.zero 8
a:
	.zero 4
b:
	.zero 4
