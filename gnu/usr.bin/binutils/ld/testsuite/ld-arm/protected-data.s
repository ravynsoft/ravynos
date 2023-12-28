	.syntax unified

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
	.thumb
	.thumb_func
	.type	getaddr, %function
getaddr:
	ldr	r3, 2f
	ldr	r2, 2f+4
1:
	add	r3, pc
	ldr	r0, [r3, r2]
	bx	lr
	.align	2
2:
	.word	_GLOBAL_OFFSET_TABLE_-(1b+4)
	.word	var(GOT)
	.size	getaddr, .-getaddr
