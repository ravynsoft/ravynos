

	.text
	.align 4
	
	.globl _foo1
_foo1: nop

	.globl _aaa2
_aaa2:
	.globl _bbb2
	.private_extern _bbb2
_bbb2:
_ccc2:
	nop
	
	.globl _bbb3
_aaa3:
_bbb3:
_ccc3:
	nop


_aaa4:
	nop

#if SUBSECTIONS
	.subsections_via_symbols
#endif
