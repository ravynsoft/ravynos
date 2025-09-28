	.syntax unified
	.thumb
three:
	bl far_fn1
	bl far_fn2
	.rept 1016
	.long 0
	.endr
	nop
label1:	
	eor.w   r0, r1, r2
	beq.w     label1

	eor.w   r0, r1, r2
 
	eor.w   r0, r1, r2
	b.w     label1

	eor.w   r0, r1, r2
	eor.w   r0, r1, r2
