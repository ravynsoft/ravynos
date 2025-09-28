	.cpu generic
	.global	ff
	.section	.tbss,"awT",%nobits
	.align	3
	.type	ff, %object
	.size	ff, 562949953421312
ff:
	.zero	562949953421312
	.global	i
	.align	2
	.type	i, %object
	.size	i, 4
i:
	.zero	4
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	movz	x0, #:tprel_g2:i
	ret
	.size	main, .-main
