	.section ".note.3", "a"
	.p2align 2
	.long .L1 - .L0		/* name length.  */
	.long .L3 - .L1		/* data length.  */
	.long  12345		/* note type.  */
.L0:
	.asciz "GNU"		/* vendor name.  */
.L1:
	.p2align 2
	.long 0			/* pr_type.  */
	.long .L5 - .L4		/* pr_datasz.  */
.L4:
	.zero 0x10
.L5:
	.p2align 2
.L3:
