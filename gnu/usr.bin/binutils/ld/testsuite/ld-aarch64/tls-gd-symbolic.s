	# Demonstrating TLS relaxation behavior for -Bsymbolic linking.
	# (i.e. no extra relaxation when using -Bsymbolic).
        .global tlsdescvar
        .global tlsdescvar2
        .section        .tbss,"awT",%nobits
        .align  2
        .type   tlsdescvar, %object
        .size   tlsdescvar, 4
tlsdescvar:
        .zero   4
        .type   tlsdescvar2, %object
        .size   tlsdescvar2, 4
tlsdescvar2:
        .zero   4
.text
_test_tls_desc:

        adrp  x0, :tlsdesc:tlsdescvar
        ldr   x1, [x0, :tlsdesc_lo12:tlsdescvar]
        add   x0, x0, :tlsdesc_lo12:tlsdescvar
        .tlsdesccall tlsdescvar
        blr   x1

_test_tls_desc2:

        adrp    x0, :gottprel:tlsdescvar2
        ldr     x0, [x0, #:gottprel_lo12:tlsdescvar2]
