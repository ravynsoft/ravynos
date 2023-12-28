        .global var

	.text
test:
        ldr	x1, :tlsdesc:var
        adr	x0, :tlsdesc:var
	.tlsdesccall var
        blr	x1
