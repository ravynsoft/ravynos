.LTLS0:
	lrw a3, xxx@GOTTPOFF
	grs a2, .LTLS0
	addu a3, a3, a2
	ld.w a3, (a3, 0)
	mov a2, tls
	str.w a0, (a2, a3 << 0)
