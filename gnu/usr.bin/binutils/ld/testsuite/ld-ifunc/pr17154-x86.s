	.text
	.globl	fct1
	.type	fct1, @gnu_indirect_function
	.set	fct1,resolve1
	.hidden int_fct1
	.globl	int_fct1
	.set	int_fct1,fct1
	.type	resolve1, @function
resolve1:
	call	func1@PLT
	.globl	g1
	.type	g1, @function
g1:
	jmp	int_fct1@PLT

	.globl	fct2
	.type	fct2, @gnu_indirect_function
	.set	fct2,resolve2
	.hidden int_fct2
	.globl	int_fct2
	.set	int_fct2,fct2
	.type	resolve2, @function
resolve2:
	call	func2@PLT
	.globl	g2
	.type	g2, @function
g2:
	jmp	int_fct2@PLT
