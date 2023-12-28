	.text
	.type ifunc, @gnu_indirect_function
	.hidden ifunc
ifunc:
	ret
	.size	ifunc, .-ifunc
	.type bar, @function
	.globl bar
bar:
        adrp    x0, :got:ifunc
        ldr     x0, [x0, #:got_lo12:ifunc]
	ret
	.size	bar, .-bar
