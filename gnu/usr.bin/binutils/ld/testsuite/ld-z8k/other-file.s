	.text

	.globl	other_start, other_label, other_imm4_val
other_start:
	nop

	djnz	r1,other_start
	dbjnz	rl0,this_label

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

other_label:
	ldb	rl2,#this_data
	ldb	rl2,#this_data + 0x10
	ldb	rl2,#other_data
	ldb	rl2,#other_data + 0x10

!	ldr	r2,this_data
!	ldr	r2,other_data

	ldk	r5,#this_imm4_val

	nop

	.set	other_imm4_val, 3

	.data
	.globl	other_data

	.ds.l	1
other_data:
	.word	0x5678
	.ds.l	1


	.end
