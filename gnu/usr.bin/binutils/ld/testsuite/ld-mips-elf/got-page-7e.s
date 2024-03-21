	# Make sure the loadable size of the library is large.
	.section .bss
	.globl	g
	.hidden	g
g:
	.space	0x800000
