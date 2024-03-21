	.syntax unified
	.arch armv7
	.thumb
main:

@These branches are to see the labels in the generated file
	bl .L888
	bl .L111
	bl .L777

@No IT block here:
	bne .L4

@The following groups should be an IT block each.
@it ne
	addne.n   pc, r0

@it ne
	tbbne [r0, r1]

@it eq
	tbheq [r1, r0]

@The following group should be left as is:
	itet	eq
.L111:	moveq	r0, #2
	movne   r0, #3
	moveq   r0, #4

@Same, reverted condition:
	itet	ne
	movne	r0, #2
	moveq   r0, #3
	movne   r0, #4


@Two groups shall be generated, due to the label:
    movne   r0, #1
@ second group, the label should be at the IT insn
.L777:	moveq	r0, #2
	ldrne   pc, [r1]

@it ne
	blne .L4
    
@it lt
	bllt .L9

@itett ne
.L888:	movne   r0, #45
	moveq   r0, #5
	movne	r0, #6
	addne.n pc, r0

@iteet eq
	moveq   r0, #7
	movne	r0, #8
	movne   r0, #3
	moveq	r0, #4

@itete eq
	moveq   r0, #5
	movne	r0, #6
	moveq   r0, #7
	movne	r0, #8

@ite eq - this group finishes due to the mov.n pc, rn
	moveq   r0, #5
	movne	r0, #6
	mov.n   pc, r0

@itete eq
	moveq   r0, #7
	movne	r0, #8
	moveq   r0, #5
	movne	r0, #6

@this shall not generate an IT block
	add.n   pc, r0

@ite eq - testing condition change (eq -> gt)
	moveq   r0, #7
	movne	r0, #8

@ite gt (group shall finish due to another condition change)
	movgt	r0, #9
	movle	r0, #10

@it eq
	moveq	r0, #11

@it le
	movle	r0, #12

@it ne
	movne	r0, #13

	bl	f
.L4:
	pop	{r4, pc}
.L9:
	bl	f

@Only the movlt shall be enclosed in the IT block
movlt r0, #0
muls r0, r0, r1

@Same here:
movlt r0, #0
muls r0, r0, r1
