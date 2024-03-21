        .section .tbss,"awT",%nobits
        .align  2
        .type   var, %object
        .size   var, 4
var:
	.zero   4

	.text
test:
        ldr	x1, :tlsdesc:var
        adr	x0, :tlsdesc:var
	.tlsdesccall var
        blr	x1
