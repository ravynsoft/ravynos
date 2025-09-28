.LTLS0:
	lrw a0, xxx@TLSLDM32
	grs a2, .LTLS0
	addu a0, a0, a2
	lrw a3, __tls_get_addr@PLT
	ldr.w a3, (gb, a3 << 0)
	jsr a3
	lrw a3, xxx@TLSLDO32
	str.w l0, (a0, a3 << 0)
