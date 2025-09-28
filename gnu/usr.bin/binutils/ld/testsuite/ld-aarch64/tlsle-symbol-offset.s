	.global	p
	.global	a
	.section	.tbss,"awT",%nobits
p:
	.zero   4096
a:
	.zero	52

	.text

# Compute the address of an integer within structure a, padded
# by an array of size 48

	mrs	x0, tpidr_el0
	add	x0, x0, #:tprel_hi12:a+48
	add	x0, x0, #:tprel_lo12_nc:a+48
	ret
