	.section .debug_info,"",@progbits
	.hidden t.c.4903c230
	.globl t.c.4903c230
t.c.4903c230:
	.byte 0x28

	.section ".note.gnu.property", "a"
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
	.long 1f - 0f		/* name length.  */
	.long 3f - 2f		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.long 5			/* note type.  */
0:	.asciz "GNU"		/* vendor name.  */
1:
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
2:
	/* GNU_PROPERTY_NO_COPY_ON_PROTECTED */
	.long 2			/* pr_type.  */
	.long 0			/* pr_datasz.  */
.ifdef __64_bit__
	.p2align 3
.else
	.p2align 2
.endif
3:
