	.text
	.global	func
	.type	func, %function
func:
	adrp	x0, :got:foo
	ldr	x0, [x0, #:got_lo12:foo]
	ldr	w0, [x0]
	ret
	.size	func, .-func
