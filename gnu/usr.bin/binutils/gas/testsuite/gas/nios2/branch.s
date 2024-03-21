# Source file used to test the beq macro.
	.globl	text_label	
	.text
.set norelax
text_label:	
	beq	r4,r5,text_label
	bge	r4,r5,text_label
	bgeu	r4,r5,text_label
	blt	r4,r5,text_label
	bltu	r4,r5,text_label
	bne	r4,r5,text_label

# Branch to an external label.
	br	external_label

