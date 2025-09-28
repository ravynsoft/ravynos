        .section .tbss,"awT",%nobits
        .align  2
        .type   var, %object
        .size   var, 4
var:
	.zero   4

	.text
test:
        adr x0, :tlsgd:var
        bl   __tls_get_addr
	nop
