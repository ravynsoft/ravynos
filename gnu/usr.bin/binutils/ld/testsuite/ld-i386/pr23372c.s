	.section ".note.gnu.property", "a"
	.p2align 2
	.long 1f - 0f		/* name length.  */
	.long 4f - 1f		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.long 5			/* note type.  */
0:
	.asciz "GNU"		/* vendor name.  */
1:
	.p2align 2
	/* GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED */
	.long 0xc0000001	/* pr_type.  */
	.long 3f - 2f		/* pr_datasz.  */
2:
	.long 0x0
3:
	.p2align 2
4:
