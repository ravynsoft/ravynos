	.text
	.globl __tls_get_addr
	.type	__tls_get_addr, @function
__tls_get_addr:
	ret
	.size	__tls_get_addr, .-__tls_get_addr
.globl _start
	.type	_start, @function
_start:
	movq	foo3@GOTTPOFF(%rip), %rax
	pushq	%rbx
	movl	%fs:foo2@TPOFF, %ebx
	addl	%fs:foo1@TPOFF, %ebx
	addl	%fs:(%rax), %ebx
	leaq	foo4@TLSLD(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	addl	foo4@DTPOFF(%rax), %ebx
	.byte	0x66
	leaq	foo5@TLSGD(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	addl	(%rax), %ebx
	movl	%ebx, %eax
	popq	%rbx
	ret
	.size	_start, .-_start
.globl foo1
	.section	.tbss,"awT",@nobits
	.align 4
	.type	foo1, @object
	.size	foo1, 4
foo1:
	.zero	4
.globl foo2
	.align 4
	.type	foo2, @object
	.size	foo2, 4
foo2:
	.zero	4
.globl foo3
	.align 4
	.type	foo3, @object
	.size	foo3, 4
foo3:
	.zero	4
.globl foo4
	.align 4
	.type	foo4, @object
	.size	foo4, 4
foo4:
	.zero	4
.globl foo5
	.align 4
	.type	foo5, @object
	.size	foo5, 4
foo5:
	.zero	4
