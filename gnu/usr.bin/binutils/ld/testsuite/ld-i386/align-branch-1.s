	.text
	.globl _start
_start:
	testl   %edx, %edx
	je	.L1
	testl   %edx, %edx
	je	.L1
	testl   %edi, %edi
	je	.L1
	leal	bar@tlsldm(%ebx), %eax
	call	___tls_get_addr@PLT
	movl	bar@dtpoff(%eax), %edx
	testl   %edx, %edx
	je	.L1
.L1:
	ret
	.section ".tdata", "awT", @progbits
bar:
	.long 10
