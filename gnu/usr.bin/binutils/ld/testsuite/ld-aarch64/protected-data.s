	.data
	.protected	var
	.global	var
	.align	2
	.type	var, %object
	.size	var, 4
var:
	.word	1

	.text
	.align	2
	.global	getaddr
	.type	getaddr, %function
getaddr:
	adrp	x0, :got:var
	ldr	x0, [x0, #:got_lo12:var]
	ret
	.size	getaddr, .-getaddr
