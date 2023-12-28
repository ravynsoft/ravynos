	.file	"a.c"
	.section	.text
	.align	3
	.global	x
	.type	x, @function
x:
	addi	sp, sp, -8
	stw	fp, 4(sp)
	mov	fp, sp
	mov	r3, zero
	.align	5
.L6:
	addi	r3, r3, 1
	cmplti	r2, r3, 100
	bne	r2, zero, .L6
	ldw	fp, 4(sp)
	addi	sp, sp, 8
	ret	
	.size	x, .-x
	.ident	"GCC: (GNU) 3.3.3 (Altera Nios II 1.0 b302)"
