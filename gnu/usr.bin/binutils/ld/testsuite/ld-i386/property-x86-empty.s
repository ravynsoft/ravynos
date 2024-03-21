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
	.long 0x0
4:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
5:
