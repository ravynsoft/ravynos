	.text
	.global start	/* Used by SH targets.  */
start:
	.global _start
_start:
	.global __start
__start:
	.global main	/* Used by HPPA targets.  */
main:
	.globl	_main	/* Used by LynxOS targets.  */
_main:
	.dc.a 0

	/* NB: Deliberately incorrect section name.  Should be
	       .note.gnu.property.  */
	.section .note, "a"
	.p2align ALIGN
	.dc.l .L1 - .L0		/* name length.  */
	.dc.l .L3 - .L1		/* data length.  */
	/* NT_GNU_PROPERTY_TYPE_0 */
	.dc.l 5			/* note type.  */
.L0:
	.asciz "GNU"		/* vendor name.  */
.L1:
	.p2align ALIGN
	/* GNU_PROPERTY_NO_COPY_ON_PROTECTED */
	.dc.l 2			/* pr_type.  */
	.dc.l .L5 - .L4		/* pr_datasz.  */
.L4:
.L5:
	.p2align ALIGN
.L3:
