	.text
	.p2align 4,,15
	.globl	foo
	.type	foo, @function
foo:
.LFB0:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	call	bar2@PLT
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	bar1@PLT
	.cfi_endproc
.LFE0:
	.size	foo, .-foo
	.section	.note.GNU-stack,"",@progbits

	.section ".note.gnu.property", "a"
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
	.long 1f - 0f		/* name length */
	.long 5f - 2f		/* data length */
	.long 5			/* note type */
0:	.asciz "GNU"		/* vendor name */
1:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
2:	.long 0xc0000002	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0x1
4:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
5:
