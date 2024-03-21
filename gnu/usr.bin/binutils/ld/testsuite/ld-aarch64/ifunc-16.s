	.text
	.globl	fct
	.type	fct, @gnu_indirect_function
	.set	fct,resolve
	.hidden int_fct
	.globl	int_fct
	.set	int_fct,fct
	.p2align 4,,15
	.type	resolve, @function
resolve:
	bl	ifunc
	.size	resolve, .-resolve
	.globl	g
	.type	g, @function
g:
	bl	int_fct
	.size	g, .-g
