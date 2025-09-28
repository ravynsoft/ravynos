	.text
	.globl	sprintf
start:	jsr	pc,sprintf
	mov	r0,space
	mov	r00f,r1
	.data
space:	.word	0
r00f:	.word	0
	.end
