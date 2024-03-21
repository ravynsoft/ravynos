; Just something allocating 128 bytes TLS data, with the symbol being global.
	.section	.tdata,"awT",@progbits
	.p2align 2
	.global tls128
	.type	tls128, @object
tls128:
	.long	47
	.fill	124,1,0
	.size	tls128, 128
