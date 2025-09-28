        .section        .tbss,"awT",@nobits
        .align  2
        .type   y, %object
        .size   y, 4
y:
        .zero   4




        .text
        .global foo
        .type foo, @function
foo:
	# Access the local symbol with an IE access.
        mov     x0, 0
        mrs     x1, tpidr_el0
        adrp    x0, :gottprel:y
        ldr     x0, [x0, #:gottprel_lo12:y]
        ldr     w0, [x1, x0]

	# Also access the same symbol with a General Dynamic access.
	# The linker should be able to recognise that if we're already
	# accessing this symbol with an Initial Exec access then this General
	# Dynamic access could be relaxed to an Initial Exec one too.
        mrs     x1, tpidr_el0
        mov     x29, sp
        adrp    x0, :tlsdesc:y
        ldr     x2, [x0, #:tlsdesc_lo12:y]
        add     x0, x0, :tlsdesc_lo12:y
        .tlsdesccall    y
        blr     x2
        ldr     w0, [x1, x0]
        ret
