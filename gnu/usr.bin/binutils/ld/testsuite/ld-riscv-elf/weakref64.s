	.option nopic
	.text
	.align	1
	.globl	_start
	.type	_start, @function
_start:
	lui	a5,%hi(f)
	addi	a5,a5,%lo(f)
	beq	a5,zero,.L1
	lla	a5,f
	beqz	a5,.L1
	addi	sp,sp,-16
	sd	ra,8(sp)
	call	f
	ld	ra,8(sp)
	addi	sp,sp,16
	tail	f
.L1:
	ret
	.size	_start, .-_start
	.weak	f
