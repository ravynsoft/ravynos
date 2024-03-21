        .global get
        .type   get, %function
get:
.LFB0:
        mrs     x0, tpidr_el0
        add     x0, x0, #:tprel_hi12:tls, lsl #12
        add     x0, x0, #:tprel_lo12_nc:tls
	add	x0, x0, #:dtprel_hi12:dtl
	add	x0, x0, #:dtprel_lo12:dtl
        ret
.LFE0:
        .size   get, .-get

