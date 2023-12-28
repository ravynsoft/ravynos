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
	subi	sp, sp, 8
	st.w	r15, (sp)
	bsr	.LTLS0
.LTLS0:
	lrw	r7, var@GOTTPOFF
	addu	r7, r15
	ld.w	r7, (r7)
	bsr	__read_tp
	addu	r7, r7, r2
	ld.w	r2, (r7)
	ld.w	r15, (sp)
	addi	sp, sp, 8
	jmp	r15
__read_tp:
	movi	r2, 0
	jmp	r15
