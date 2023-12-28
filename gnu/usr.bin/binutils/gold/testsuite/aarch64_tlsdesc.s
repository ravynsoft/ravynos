        .global test, var
        .text
test:
        adrp    x0, :tlsdesc:var
        ldr     x1, [x0, :tlsdesc_lo12:var]
        add     x0, x0, :tlsdesc_lo12:var
        .tlsdesccall var
        blr     x1

        .section        .tbss,"awT",%nobits
var:
        .zero   4
