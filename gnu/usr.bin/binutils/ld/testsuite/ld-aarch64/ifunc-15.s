	.text
        .type foo, @function
	.global foo
foo:
	adrp    x0, :got:ifunc
        ldr     x0, [x0, #:got_lo12:ifunc]
        ret
        .type ifunc, @gnu_indirect_function
	.globl ifunc
ifunc:
        ret
