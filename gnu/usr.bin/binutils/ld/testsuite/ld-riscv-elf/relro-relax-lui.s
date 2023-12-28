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
	lui	a0, %hi (SymbolRodata)
	addi	a0, a0, %lo (SymbolRodata)
