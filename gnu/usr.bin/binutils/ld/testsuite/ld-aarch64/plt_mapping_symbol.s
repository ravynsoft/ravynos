	.text
test:
	bl   __tls_get_addr
	nop

	.xword 0xffee
