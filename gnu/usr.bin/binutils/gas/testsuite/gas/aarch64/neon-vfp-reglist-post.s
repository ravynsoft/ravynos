
	# ARMv8 tests to test neon register 
	# lists syntax.
	.text
	.arch armv8-a

	# Post-index multiple elements

	.macro ldst1_reg_list_post_imm_64 inst type
	\inst\()1 {v0.\type}, [x0], #8
	\inst\()1 {v0.\type, v1.\type}, [x0], #16
	\inst\()1 {v0.\type, v1.\type, v2.\type}, [x0], #24
	\inst\()1 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #32
	.endm
	
	.irp instr ld,st
	.irp bits_64 8b, 4h, 2s, 1d
	ldst1_reg_list_post_imm_64 \instr \bits_64
	.endr
	.endr

	.macro ldst1_reg_list_post_imm_128 inst type
	\inst\()1 {v0.\type}, [x0], #16
	\inst\()1 {v0.\type, v1.\type}, [x0], #32
	\inst\()1 {v0.\type, v1.\type, v2.\type}, [x0], #48
	\inst\()1 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #64
	.endm

	.irp instr ld,st
	.irp bits_128 16b, 8h, 4s, 2d
	ldst1_reg_list_post_imm_128 \instr \bits_128
	.endr
	.endr

	.macro ldst1_reg_list_post_reg inst type postreg
	\inst\()1 {v0.\type}, [x0], \postreg
	\inst\()1 {v0.\type, v1.\type}, [x0], \postreg
	\inst\()1 {v0.\type, v1.\type, v2.\type}, [x0], \postreg
	\inst\()1 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], \postreg
	.endm

	.irp instr ld,st
	.irp bits 8b, 4h, 2s, 1d, 16b, 8h, 4s, 2d
	ldst1_reg_list_post_reg \instr \bits x7
	.endr
	.endr

	.macro ldst2_reg_list_post_imm_reg_64 inst type postreg
	\inst\()2 {v0.\type, v1.\type}, [x0], #16
	.ifnb \postreg
	\inst\()2 {v0.\type, v1.\type}, [x0], \postreg
	.endif
	.endm

	.macro ldst2_reg_list_post_imm_reg_128 inst type postreg
	\inst\()2 {v0.\type, v1.\type}, [x0], #32
	.ifnb \postreg
	\inst\()2 {v0.\type, v1.\type}, [x0], \postreg
	.endif
	.endm

	.irp instr ld,st
	.irp bits_64 8b, 4h, 2s
	ldst2_reg_list_post_imm_reg_64 \instr \bits_64 x7
	.endr
	.endr

	.irp instr ld,st
	.irp bits_128 16b, 8h, 4s, 2d
	ldst2_reg_list_post_imm_reg_128 \instr \bits_128 x7
	.endr
	.endr

	.macro ldst34_reg_list_post_imm_reg_64 inst type postreg
	\inst\()3 {v0.\type, v1.\type, v2.\type}, [x0], #24
	\inst\()4 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #32
	\inst\()3 {v0.\type, v1.\type, v2.\type}, [x0], \postreg
	\inst\()4 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], \postreg
	.endm

	.macro ldst34_reg_list_post_imm_reg_128 inst type postreg
	\inst\()3 {v0.\type, v1.\type, v2.\type}, [x0], #48
	\inst\()4 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #64
	\inst\()3 {v0.\type, v1.\type, v2.\type}, [x0], \postreg
	\inst\()4 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], \postreg
	.endm

	.irp instr ld,st
	.irp bits_64 8b, 4h, 2s
	ldst34_reg_list_post_imm_reg_64 \instr \bits_64 x7
	.endr
	.endr

	.irp instr ld,st
	.irp bits_128 16b, 8h, 4s, 2d
	ldst34_reg_list_post_imm_reg_128 \instr \bits_128 x7
	.endr
	.endr


	# Post Index Vector-element form with replicate (Immediate offset)

	# Consecutive registers in reg list

	.macro ldstn_index_rep_B_imm inst index type rep
	\inst\()1\rep {v0.\type}\index, [x0], #1
	\inst\()2\rep {v0.\type, v1.\type}\index, [x0], #2
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0], #3
	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0], #4
	.endm

	# Vector element with index

	.irp instr, ld, st
	ldstn_index_rep_B_imm  \instr index="[1]" type=b rep=""
	.ifnc \instr, st
	.irp types 8b, 16b
	ldstn_index_rep_B_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.macro ldstn_index_rep_H_imm inst index type rep
	\inst\()1\rep {v0.\type}\index, [x0], #2
	\inst\()2\rep {v0.\type, v1.\type}\index, [x0], #4
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0], #6
	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0], #8
	.endm

	.irp instr, ld, st
	ldstn_index_rep_H_imm  \instr index="[1]" type=h rep=""
	.ifnc \instr, st
	.irp types 4h, 8h
	ldstn_index_rep_H_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.macro ldstn_index_rep_S_imm inst index type rep
	\inst\()1\rep {v0.\type}\index, [x0], #4
	\inst\()2\rep {v0.\type, v1.\type}\index, [x0], #8
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0], #12
	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0], #16
	.endm

	.irp instr, ld, st
	ldstn_index_rep_S_imm  \instr index="[1]" type=s rep=""
	.ifnc \instr, st
	.irp types 2s, 4s
	ldstn_index_rep_S_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.macro ldstn_index_rep_D_imm inst index type rep
	\inst\()1\rep {v0.\type}\index, [x0], #8
	\inst\()2\rep {v0.\type, v1.\type}\index, [x0], #16
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0], #24
	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0], #32
	.endm

	.irp instr, ld, st
	ldstn_index_rep_D_imm  \instr index="[1]" type=d rep=""
	.ifnc \instr, st
	.irp types 1d, 2d
	ldstn_index_rep_D_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	# Post Index Vector-element form with replicate (Register offset)
	# This could have been factored into Post-index multiple 
	# element macros but this would make this already-looking-complex
	# testcase look more complex!

	# Consecutive registers in reg list

	.macro ldstn_index_rep_reg inst index type rep postreg
	\inst\()1\rep {v0.\type}\index, [x0], \postreg
	\inst\()2\rep {v0.\type, v1.\type}\index, [x0], \postreg
	\inst\()3\rep {v0.\type, v1.\type, v2.\type}\index, [x0], \postreg
	\inst\()4\rep {v0.\type, v1.\type, v2.\type, v3.\type}\index, [x0], \postreg
	.endm

	.irp instr, ld, st
	.irp itypes b,h,s,d
	ldstn_index_rep_reg  \instr index="[1]" type=\itypes rep="" postreg=x7
	.endr
	.ifnc \instr, st
	.irp types 8b, 16b, 4h, 8h, 2s, 4s, 1d, 2d
	ldstn_index_rep_reg  \instr index="" type=\types rep="r" postreg=x7
	.endr
	.endif
	.endr

	# ### End of test
