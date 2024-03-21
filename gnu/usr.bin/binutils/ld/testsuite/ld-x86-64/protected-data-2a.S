	.text
	.p2align 4
	.protected	protected_data_1a
	.globl	protected_data_1a_p
	.type	protected_data_1a_p, @function
protected_data_1a_p:
.LFB0:
	.cfi_startproc
	leaq	protected_data_1a(%rip), %rax
	ret
	.cfi_endproc
.LFE0:
	.size	protected_data_1a_p, .-protected_data_1a_p
	.p2align 4
	.protected	protected_data_1b
	.globl	protected_data_1b_p
	.type	protected_data_1b_p, @function
protected_data_1b_p:
.LFB1:
	.cfi_startproc
	leaq	protected_data_1b(%rip), %rax
	ret
	.cfi_endproc
.LFE1:
	.size	protected_data_1b_p, .-protected_data_1b_p
	.p2align 4
	.globl	set_protected_data_1a
	.type	set_protected_data_1a, @function
set_protected_data_1a:
.LFB2:
	.cfi_startproc
	movl	%edi, protected_data_1a(%rip)
	ret
	.cfi_endproc
.LFE2:
	.size	set_protected_data_1a, .-set_protected_data_1a
	.p2align 4
	.globl	set_protected_data_1b
	.type	set_protected_data_1b, @function
set_protected_data_1b:
.LFB3:
	.cfi_startproc
	movl	%edi, protected_data_1b(%rip)
	ret
	.cfi_endproc
.LFE3:
	.size	set_protected_data_1b, .-set_protected_data_1b
	.p2align 4
	.globl	check_protected_data_1a
	.type	check_protected_data_1a, @function
check_protected_data_1a:
.LFB4:
	.cfi_startproc
	xorl	%eax, %eax
	cmpl	%edi, protected_data_1a(%rip)
	setne	%al
	ret
	.cfi_endproc
.LFE4:
	.size	check_protected_data_1a, .-check_protected_data_1a
	.p2align 4
	.globl	check_protected_data_1b
	.type	check_protected_data_1b, @function
check_protected_data_1b:
.LFB5:
	.cfi_startproc
	xorl	%eax, %eax
	cmpl	%edi, protected_data_1b(%rip)
	setne	%al
	ret
	.cfi_endproc
.LFE5:
	.size	check_protected_data_1b, .-check_protected_data_1b
	.globl	protected_data_1b
	.data
	.align 4
	.type	protected_data_1b, @object
	.size	protected_data_1b, 4
protected_data_1b:
	.long	2
	.globl	protected_data_1a
	.align 4
	.type	protected_data_1a, @object
	.size	protected_data_1a, 4
protected_data_1a:
	.long	1
	.section	.note.GNU-stack,"",@progbits
#ifdef USE_GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS
# ifdef __LP64__
#  define ALIGN 3
# else
#  define ALIGN 2
# endif
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
#endif
