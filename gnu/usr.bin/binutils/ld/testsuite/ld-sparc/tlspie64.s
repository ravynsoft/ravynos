	.section	.tbss,"awT",@nobits
	.global tls_gd
	.align 4
	.type	tls_gd, #object
	.size	tls_gd, 4
tls_gd:
	.skip	4
	.global tls_ld
	.align 4
	.type	tls_ld, #object
	.size	tls_ld, 4
tls_ld:
	.skip	4
	.global tls_ie
	.align 4
	.type	tls_ie, #object
	.size	tls_ie, 4
tls_ie:
	.skip	4
	.section	".text"
.LLGETPC0:
	retl
	  add     %o7, %l7, %l7
	.align 4
	.global foo
	.type	foo, #function
	.proc	0104
foo:
	save	%sp, -160, %sp
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7
	call	.LLGETPC0
	add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7
	nop;nop;nop;nop

	/* GD -> IE with global variable not defined in executable */
	sethi	%tgd_hi22(sG1), %g1
	add	%g1, %tgd_lo10(sG1), %g1
	add	%l7, %g1, %o0, %tgd_add(sG1)
	call	__tls_get_addr, %tgd_call(sG1)
	 nop
	nop;nop;nop;nop

	/* GD -> LE with global variable defined in executable */
	sethi	%tgd_hi22(tls_gd), %g1
	add	%g1, %tgd_lo10(tls_gd), %g1
	add	%l7, %g1, %o0, %tgd_add(tls_gd)
	call	__tls_get_addr, %tgd_call(tls_gd)
	 nop
	nop;nop;nop;nop

	/* LD -> LE with global variable defined in executable */
	sethi	%tldm_hi22(tls_ld), %g1
	add	%g1, %tldm_lo10(tls_ld), %g1
	add	%l7, %g1, %o0, %tldm_add(tls_ld)
	call	__tls_get_addr, %tldm_call(tls_ld)
	 nop
	sethi	%tldo_hix22(tls_ld), %g1
	xor	%g1, %tldo_lox10(tls_ld), %g1
	add	%o0, %g1, %g1, %tldo_add(tls_ld)
	nop;nop;nop;nop

	/* IE -> LE with global variable defined in executable */
	sethi	%tie_hi22(tls_ie), %g1
	add	%g1, %tie_lo10(tls_ie), %g1
	ldx	[%l7 + %g1], %g1, %tie_ldx(tls_ie)
	add	%g7, %g1, %g1, %tie_add(tls_ie)
	nop;nop;nop;nop

	ret
	restore
