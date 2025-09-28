

	.macro iterate_regs_types macro_name reg
	.irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	.irp regs, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
	\macro_name \regs b \index \reg
	.endr
	.endr

	.irp index, 0,1,2,3,4,5,6,7
	.irp regs, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
	\macro_name \regs h \index \reg
	.endr
	.endr

	.irp index, 0,1,2,3
	.irp regs, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
	\macro_name \regs s \index \reg
	.endr
	.endr
	.endm

	.macro ins_mov_main reg_num type index xw_reg 
	ins v\reg_num\().\type[\index], \xw_reg\reg_num
	mov v\reg_num\().\type[\index], \xw_reg\reg_num
	.endm

	.macro ins_mov_element reg_num type index null
	ins v\reg_num\().\type[\index], v\reg_num\().\type[\index] 
	mov v\reg_num\().\type[\index], v\reg_num\().\type[\index] 
	.endm

	.text
	iterate_regs_types macro_name=ins_mov_main reg=w
	iterate_regs_types macro_name=ins_mov_element

	.irp reg, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
	ins v\reg\().d[0], x\reg
	mov v\reg\().d[0], x\reg
	ins v\reg\().d[1], x\reg
	mov v\reg\().d[1], x\reg

	ins v\reg\().d[0], v\reg\().d[1] 
	mov v\reg\().d[0], v\reg\().d[1] 
	ins v\reg\().d[1], v\reg\().d[0] 
	mov v\reg\().d[1], v\reg\().d[0] 
	.endr
	
