	.code16

	.macro addr16 mnem:req opnds:vararg
	addr32 \mnem \opnds
	.endm

	.include "lea.s"

	lea	0xffff(%bx), %ax
	lea	-0xffff(%bx), %ax
