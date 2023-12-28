	.cpu generic
	.global	ff
	.section	.tbss,"awT",%nobits
	.align	3
	.type	ff, %object
	.size	ff, 67108864
ff:
	.zero	67108864
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
	sub	sp, sp, #16
	str	wzr, [sp,12]
	b	.L2
.L3:
	mrs	x0, tpidr_el0
	add	x1, x0, #:tprel_hi12:ff
	add	x1, x1, #:tprel_lo12_nc:ff
	ldrsw	x0, [sp,12]
	mov	w2, 7
	strb	w2, [x1,x0]
	ldr	w0, [sp,12]
	add	w0, w0, 1
	str	w0, [sp,12]
.L2:
	ldr	w0, [sp,12]
	cmp	w0, 999
	ble	.L3
	mrs	x0, tpidr_el0
	add	x0, x0, #:tprel_hi12:i
	add	x0, x0, #:tprel_lo12_nc:i
	ldr	w0, [x0]
	add	sp, sp, 16
	ret
	.size	main, .-main
