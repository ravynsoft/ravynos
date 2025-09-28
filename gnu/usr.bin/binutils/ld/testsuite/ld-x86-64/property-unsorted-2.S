#ifdef __LP64__
# define ALIGN 3
#else
# define ALIGN 2
#endif
	.section ".note.gnu.property", "a"
	.p2align ALIGN
	.long 1f - 0f		/* name length.  */
	.long 3f - 1f		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.long 5			/* note type.  */
0:
	.asciz "GNU"		/* vendor name.  */
1:
	.p2align ALIGN
	/* GNU_PROPERTY_NO_COPY_ON_PROTECTED */
	.long 2			/* pr_type.  */
	.long 0			/* pr_datasz.  */
	.p2align ALIGN
	/* GNU_PROPERTY_STACK_SIZE */
	.long 1			/* pr_type.  */
	.long 5f - 4f		/* pr_datasz.  */
4:
	.dc.a 0x800000		/* Stack size.  */
5:
	.p2align ALIGN
3:
	.section	.note.GNU-stack
