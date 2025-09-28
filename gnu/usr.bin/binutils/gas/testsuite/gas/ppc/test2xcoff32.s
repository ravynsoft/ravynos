	.csect .text[PR]
	.extern .printf
	.toc
LC..1:
	.tc LC..0[TC],LC..0
	.csect .text[PR]
	.align 2
	.globl main
	.globl .main
	.csect main[DS]
main:
	.long .main, TOC[tc0], 0
	.csect .text[PR]
.main:
	mflr 0
	stw 0,8(1)
	stw 30,-8(1)
	stw 31,-4(1)
	stwu 1,-64(1)
	li 31,0
	lwz 30,LC..1(2)
L..2:
	mr 3,30
	mr 4,31
	bl .printf
	nop
	cmpwi 7,31,9
	addi 31,31,1
	bne 7,L..2
	li 3,0
	addi 1,1,64
	lwz 0,8(1)
	mtlr 0
	lwz 30,-8(1)
	lwz 31,-4(1)
	blr
	
	.csect _t.rw_[RO],4
	.align 2
LC..0:
	.byte "%d"
	.byte 10, 0
