        .section .text.foo,"ax",@progbits
        .type foo, @function
foo:
        .global foo
        adrp    x0, :got:ifunc
        ldr     x0, [x0, #:got_lo12:ifunc]
	bl ifunc
        adrp    x0, xxx
        add     x0, x0, :lo12:xxx
        ret

        .section .text.bar,"ax",@progbits
        .type bar, @function
bar:
        .global bar
        ret

        .section .text.ifunc,"ax",@progbits
        .type ifunc, @gnu_indirect_function
ifunc:
        ret

        .section .data.foo,"aw",@progbits
xxx:
	.quad ifunc 
