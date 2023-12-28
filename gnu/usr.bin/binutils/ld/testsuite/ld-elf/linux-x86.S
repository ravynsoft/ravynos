	.text
	.globl _start
	.type _start,@function
	.p2align 4
_start:
	xorl %ebp, %ebp
#ifdef __LP64__
	popq %rdi
	movq %rsp, %rsi
	andq  $~15, %rsp
#elif defined __x86_64__
	mov (%rsp),%edi
	addl $4,%esp
	movl %esp, %esi
	andl  $~15, %esp
#else
	popl %esi
	movl %esp, %ecx
	andl  $~15, %esp

	subl $8,%esp
	pushl %ecx
	pushl %esi
#endif

	call main

	hlt

	.type syscall,  @function
	.globl syscall
	.p2align 4
syscall:
#ifdef __x86_64__
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	movq %r9, %r8
	movq 8(%rsp),%r9	/* arg6 is on the stack.  */
	syscall			/* Do the system call.  */
#else
	push %ebp
	push %edi
	push %esi
	push %ebx
	mov 0x2c(%esp),%ebp
	mov 0x28(%esp),%edi
	mov 0x24(%esp),%esi
	mov 0x20(%esp),%edx
	mov 0x1c(%esp),%ecx
	mov 0x18(%esp),%ebx
	mov 0x14(%esp),%eax
	int $0x80
	pop %ebx
	pop %esi
	pop %edi
	pop %ebp
#endif
	ret			/* Return to caller.  */
	.size syscall, .-syscall
	.section .note.GNU-stack,"",@progbits

	.section ".note.gnu.property", "a"
#ifdef __LP64__
	.p2align 3
#else
	.p2align 2
#endif
	.long 1f - 0f		/* name length */
	.long 5f - 2f		/* data length */
	.long 5			/* note type */
0:	.asciz "GNU"		/* vendor name */
1:
#ifdef __LP64__
	.p2align 3
#else
	.p2align 2
#endif
2:	.long 0xc0000002	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0x2
4:
#ifdef __LP64__
	.p2align 3
#else
	.p2align 2
#endif
5:
