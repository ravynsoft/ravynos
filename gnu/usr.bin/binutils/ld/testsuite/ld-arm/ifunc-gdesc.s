
	.arm

foo:
	bl	ifunc1(PLT)
	ldr	r0,1f
2:	bl	loc1(tlscall)
	nop
1:	.word	loc1(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	loc2(tlscall)
	nop
1:	.word	loc2(tlsdesc) + (. - 2b)

	.type	ifunc1,%gnu_indirect_function
	.global ifunc1
ifunc1:
	mov	pc,lr
	.size	ifunc1,.-ifunc1


	.section	.tdata,"awT",%progbits
	.space	8
	.type	loc1, %object
loc1:	.space	4
	.type	loc2, %object
loc2:	.space	4

