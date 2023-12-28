	.text
	.section	.tbss,"awT",@nobits
	.align	2
	.type	var, @object
	.size	var, 4
var:
	.word	0

	.text
	.align	2
	.global	_start
	.type	_start, @function
_start:
	subi	sp, sp, 4
	st.w	l4, (sp, 0)
	mov	l4, sp
.LTLS0:
	lrw	a3, var@GOTTPOFF
	grs	a2, .LTLS0
	addu	a3, a3, a2
	ld.w	a3, (a3, 0)
	ldr.w	a3, (r31, a3 << 0)
	mov	a0, a3
	mov	sp, l4
	ld.w	l4, (sp, 0)
	addi	sp, sp, 4
	rts
