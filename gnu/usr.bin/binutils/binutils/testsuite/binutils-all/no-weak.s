	.file	"no-weak.c"
	.text

	.globl	weak_with_default_value
	.weak   weak_with_default_value
weak_with_default_value:	
	.nop
	
	.data
	.weak   weak_without_default_value
	.dc.a   weak_without_default_value
	
	
