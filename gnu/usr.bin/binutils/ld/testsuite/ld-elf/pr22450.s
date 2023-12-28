
	.section ".note.gnu", "a" /* NB/ Deliberately incorrect section name.  Should be: .note.gnu.property  */
	.p2align ALIGN

	.dc.l 1f - 0f		/* Name length.  */
	.dc.l 5f - 2f		/* Data length.  */
	.dc.l 5			/* Note type: NT_GNU_PROPERTY_TYPE_0 */
0:
	.asciz "GNU"		/* Vendor name.  */
1:
	.p2align ALIGN
2:	
	.dc.l 1			/* pr_type: GNU_PROPERTY_STACK_SIZE */
	.dc.l 5f - 4f		/* pr_datasz.  */
4:
	.dc.a 0x8000		/* Stack size.  */
5:
	.p2align ALIGN
3:
