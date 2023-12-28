	.cpu generic
	.global	ff
	.section        .tdata,"awT",%progbits
	.align	2
	.type	ff, %object
         # Maximum 12bit - 16byte TCB header is the upper limit
	 # for tprel_add_lo12
	.size	ff, 4096 - 16
ff:
	.zero	4096 - 16
	.global	i
	.type	i, %object
	.size	i, 4
i:
	.zero	4
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	add	x0, x0, #:tprel_lo12:i
	ret
	.size	main, .-main
