	.text
	.globl ___tls_get_addr
	.type	___tls_get_addr, @function
___tls_get_addr:
	ret
	.size	___tls_get_addr, .-___tls_get_addr
.globl _start
	.type	_start, @function
_start:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	call	.L3
.L3:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L3], %ebx
	movl	%gs:foo2@NTPOFF, %esi
	addl	%gs:foo1@NTPOFF, %esi
	movl	foo3@GOTNTPOFF(%ebx), %eax
	addl	%gs:(%eax), %esi
	leal	foo4@TLSGD(,%ebx,1), %eax
	call	___tls_get_addr@PLT
	addl	(%eax), %esi
	leal	foo5@TLSGD(,%ebx,1), %eax
	call	___tls_get_addr@PLT
	addl	(%eax), %esi
	movl	%esi, %eax
	popl	%ebx
	popl	%esi
	leave
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
