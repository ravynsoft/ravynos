
	.section .text
text:
	brr	tr,label
	nop
	moviu	r6, %u label
	movil	r6, %l label
	bra	tr,r6,r0
	nop

	.section .text2
text2:
	nop
	nop
	nop
	nop
label:
	.end


