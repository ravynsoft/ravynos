        .section        .text
        .align  2
        .global foo
        .type   foo, @function
foo:
        nop.n
        .section        .text
        .size   foo, .-foo
