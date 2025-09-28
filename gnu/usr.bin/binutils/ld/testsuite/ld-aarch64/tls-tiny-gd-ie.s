	.text
test:
        adr x0, :tlsgd:var
        bl   __tls_get_addr
	nop
