	.text
	.p2align 4
	.protected	protected_func_1a
	.globl	protected_func_1a
	.type	protected_func_1a, @function
protected_func_1a:
.LFB0:
	.cfi_startproc
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	protected_func_1a, .-protected_func_1a
	.p2align 4
	.protected	protected_func_1b
	.globl	protected_func_1b
	.type	protected_func_1b, @function
protected_func_1b:
.LFB1:
	.cfi_startproc
	movl	$2, %eax
	ret
	.cfi_endproc
.LFE1:
	.size	protected_func_1b, .-protected_func_1b
	.p2align 4
	.globl	protected_func_1a_p
	.type	protected_func_1a_p, @function
protected_func_1a_p:
.LFB2:
	.cfi_startproc
	leaq	protected_func_1a(%rip), %rax
	ret
	.cfi_endproc
.LFE2:
	.size	protected_func_1a_p, .-protected_func_1a_p
	.p2align 4
	.globl	protected_func_1b_p
	.type	protected_func_1b_p, @function
protected_func_1b_p:
.LFB3:
	.cfi_startproc
	leaq	protected_func_1b(%rip), %rax
	ret
	.cfi_endproc
.LFE3:
	.size	protected_func_1b_p, .-protected_func_1b_p
	.section	.note.GNU-stack,"",@progbits
#ifdef __LP64__
# define ALIGN 3
#else
# define ALIGN 2
#endif
	.section ".note.gnu.property", "a"
	.p2align ALIGN
	.long 1f - 0f		/* name length */
	.long 5f - 2f		/* data length */
	.long 5			/* note type */
0:	.asciz "GNU"		/* vendor name */
1:
	.p2align ALIGN
2:	.long 0xb0008000	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0x1
4:
	.p2align ALIGN
5:
