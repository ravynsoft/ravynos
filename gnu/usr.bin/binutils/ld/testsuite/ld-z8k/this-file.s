	.text

	.globl	this_start, this_label, this_imm4_val
this_start:
	nop

	jr	t,this_start
	jr	t,other_start

	jp	t,this_start
	jp	t,other_start

	call	other_start
	call	other_label
	call	this_start
	call	this_label

	calr	other_start
	calr	other_label
	calr	this_start
	calr	this_label

	ld	r1,other_data
	ld	r1,this_data
	ld	r1,#other_data
	ld	r1,#this_data

this_label:
	ldb	rl2,#this_data
	ldb	rl2,#this_data + 0x10
	ldb	rl2,#other_data
	ldb	rl2,#other_data + 0x10

!	ldr	r2,this_data
!	ldr	r2,other_data

	bitb	rl3,#other_imm4_val
	nop

	.set	this_imm4_val, 10

	.data

	.ds.l	1
	.globl	this_data
this_data:
	.word	0x1234
	.ds.l	1

	.end
