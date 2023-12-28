	.global	foo
	.type	foo, %function
foo:
	sub	sp, sp, #16
	mov	w0, 9
	str	w0, [sp, 12]
	ldr	w0, [sp, 12]
	add	w0, w0, 4
	str	w0, [sp, 12]
	nop
	add	sp, sp, 16
	ret
	.size	foo, .-foo
	.global	bar
	.type	bar, %function
bar:
	sub	sp, sp, #16
	mov	w0, 9
	str	w0, [sp, 12]
	ldr	w0, [sp, 12]
	add	w0, w0, 4
	str	w0, [sp, 12]
	nop
	add	sp, sp, 16
	ret
	.size	bar, .-bar
	.section ".note.gnu.property", "a"
	.p2align 3
	.long 1f - 0f		/* name length */
	.long 5f - 2f		/* data length */
	.long 5			/* note type */
0:	.asciz "GNU"		/* vendor name */
1:
	.p2align 3
2:	.long 0xc0000000	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0x1		/* BTI.  */
4:
	.p2align 3
5:
