	.text
	.global _start
_start:
	.long 1

	.section ".note.ABI-tag", "a"
	.p2align 2
	.long 1f - 0f		/* name length */
	.long 3f - 2f		/* data length */
	.long  1		/* note type */
0:	.asciz "GNU"		/* vendor name */
1:	.p2align 2
2:	.long 1
	.long 2
3:	.p2align 2		/* pad out section */
