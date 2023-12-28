	.text
	.globl _start
_start:
	testl   %edx, %edx
	je	.L1
	testq   %rdi, %rdi
	je	.L1
	leaq	bar@tlsld(%rip), %rdi
	call	__tls_get_addr@PLT
	movq	bar@dtpoff(%rax), %rbx
	testq   %rbx, %rbx
	je	.L1
.L1:
	ret
	.section ".tdata", "awT", @progbits
bar:
	.long 10
