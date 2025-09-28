	.global	gdesc_var_1
	.global	gd_var_1
	.section	.tdata,"awT",%progbits
gdesc_var_1:
	.word	1
gdesc_var_2:
	.word	2
gd_var_1:
	.word	3
gd_var_2:
	.word	4
ie_var:
	.word	5
	.text
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	adrp	x0, :tlsdesc:gdesc_var_1
	ldr	x1, [x0, #:tlsdesc_lo12:gdesc_var_1]
	add	x0, x0, :tlsdesc_lo12:gdesc_var_1
	.tlsdesccall	gdesc_var_1
	blr	x1
	mrs	x1, tpidr_el0
	add	x0, x1, x0
	ldr	w1, [x0]
	adrp	x0, :tlsdesc:gdesc_var_2
	ldr	x2, [x0, #:tlsdesc_lo12:gdesc_var_2]
	add	x0, x0, :tlsdesc_lo12:gdesc_var_2
	.tlsdesccall	gdesc_var_2
	blr	x2
	mrs	x2, tpidr_el0
	add	x0, x2, x0
	ldr	w0, [x0]
	add	w1, w1, w0
	adrp	x0, :tlsgd:gd_var_1
	add	x0, x0, :tlsgd_lo12:gd_var_1
	bl	__tls_get_addr
	nop
	ldr	w0, [x0]
	add	w1, w1, w0
	adrp	x0, :tlsgd:gd_var_2
	add	x0, x0, :tlsgd_lo12:gd_var_2
	bl	__tls_get_addr
	nop
	ldr	w0, [x0]
	add	w1, w1, w0
	mrs	x2, tpidr_el0
	adrp	x0, :gottprel:ie_var
	ldr	x0, [x0, #:gottprel_lo12:ie_var]
	add	x0, x2, x0
	ldr	w0, [x0]
	add	w0, w1, w0
