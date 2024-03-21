	.text
	.type ifunc, @gnu_indirect_function
	.global ifunc
ifunc:
	ret
	.size	ifunc, .-ifunc
	.type _start, @function
	.globl _start
_start:
        adrp    x0, :got:ifunc
        ldr     x0, [x0, #:got_lo12:ifunc]
	.size	_start, .-_start
	.data
	.xword	ifunc
