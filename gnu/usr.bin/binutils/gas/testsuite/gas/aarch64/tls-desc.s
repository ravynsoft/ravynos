func:

	// R_AARCH64_TLSDESC_0FF_G1  var
	movz  x0, #:tlsdesc_off_g1:var
	// R_AARCH64_TLSDESC_OFF_G0_NC var
	movk  x0, #:tlsdesc_off_g0_nc:var

	.tlsdescldr var
	// R_AARCH64_TLSDESC_LDR  var
	ldr   x1, [x18, x0]
	.tlsdescadd var
	// R_AARCH64_TLSDESC_ADD  var
	add   x0, x18, x0
	.tlsdesccall var
	// R_AARCH64_TLSDESC_CALL  var
	blr   x1
