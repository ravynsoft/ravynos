	.section ".note.gnu.property", "a"
	.p2align 2
	.long 1f - 0f		/* name length.  */
	.long 5f - 2f		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.long 5			/* note type.  */
0:	.asciz "GNU"		/* vendor name.  */
1:	.p2align 2
2:
	/* GNU_PROPERTY_STACK_SIZE */
	.long 1			/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.dc.a 0x800000		/* Stack size.  */
4:
	.p2align 2
	/* GNU_PROPERTY_NO_COPY_ON_PROTECTED */
	.long 2			/* pr_type.  */
	.long 0			/* pr_datasz.  */
	.p2align 2
	/* GNU_PROPERTY_X86_ISA_1_USED */
	.long 0xc0010002	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0x7fffffff
4:
	.p2align 2
	/* GNU_PROPERTY_X86_ISA_1_NEEDED */
	.long 0xc0008002	/* pr_type.  */
	.long 4f - 3f		/* pr_datasz.  */
3:
	.long 0xffff
4:
	.p2align 2
5:
