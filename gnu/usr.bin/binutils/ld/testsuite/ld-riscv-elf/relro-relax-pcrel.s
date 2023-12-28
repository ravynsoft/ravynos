	.section	.rodata
	.align	10
	.globl	SymbolRodata
	.set	SymbolRodata, . + 0x1800
	.word	0x0


	.section	.init_array
        .word	0x0

	.text
	.globl	_start
_start:
	lla	a0, SymbolRodata
