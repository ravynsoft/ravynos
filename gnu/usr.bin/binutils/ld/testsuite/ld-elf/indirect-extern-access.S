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
