	.section	.tbss,"awT",@nobits
	.align	2
	.type	var, @object
	.size	var, 4
var:
	.fill 4, 1
	.text
	.align	2
	.global	_start
	.type	_start, @function
_start:
	subi	sp, sp, 8
	st.w	r15, (sp)
	st.w	r8, (sp, 4)
	mov	r8, sp
	bsr	__read_tp
	mov	r6, r2
	lrw	r7, var@TPOFF
	addu	r7, r7, r6
	ld.w	r7, (r7)
	mov	r2, r7
	mov	sp, r8
	ld.w	r15, (sp)
	ld.w	r8, (sp, 4)
	addi	sp, sp, 8
	jmp	r15
__read_tp:
	movi	r2, 0
	jmp	r15
