	.section ".note.gnu.property", "a"
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
	.long 1f - 0f		/* name length.  */
	.long 4f - 1f		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.long 5			/* note type.  */
0:
	.asciz "GNU"		/* vendor name.  */
1:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
	/* GNU_PROPERTY_X86_ISA_1_NEEDED */
	.long 0xc0008002	/* pr_type.  */
	.long 3f - 2f		/* pr_datasz.  */
2:
	.long 0x3
3:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
4:
