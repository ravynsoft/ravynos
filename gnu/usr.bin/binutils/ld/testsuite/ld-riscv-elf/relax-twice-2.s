	.file	"<artificial>"
	.option pic
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"%u"
	.text
	.align	1
	.globl	relaxme
	.type	relaxme, @function
relaxme:
	addi	sp,sp,-32
	addi	a2,sp,12
	lla	a1,.LC0
	li	a0,0
	sd	ra,24(sp)
	call	sscanf@plt
	ld	ra,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	relaxme, .-relaxme
	.align	1
	.globl	foobar_new
	.type	foobar_new, @function
foobar_new:
	li	a0,1
	ret
	.size	foobar_new, .-foobar_new
	.symver	foobar_new, foobar@@New
	.align	1
	.globl	foobar_old
	.type	foobar_old, @function
foobar_old:
	addi	sp,sp,-16
	sd	ra,8(sp)
	call	foobar@plt
	ld	ra,8(sp)
	snez	a0,a0
	addi	sp,sp,16
	jr	ra
	.size	foobar_old, .-foobar_old
	.symver	foobar_old, foobar@Old
	.section	.note.GNU-stack,"",@progbits
