	.section .data.rel,"aw",@progbits
	.globl ifunc_ptrt
	.type	ifunc_ptr, @object
ifunc_ptr:
	.dc.a ifunc
	.text
	.type ifunc, @gnu_indirect_function
	.globl ifunc
ifunc:
	ret
	.size	ifunc, .-ifunc
	.type bar, @function
	.globl bar
bar:
	call	ifunc@PLT
	.size	bar, .-bar
