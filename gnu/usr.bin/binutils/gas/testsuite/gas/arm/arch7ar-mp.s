	@ Test MP Extension instructions
	.text

label1:
	pldw [r0, #0]
	pldw [r14, #0]
	pldw [r1, #0]
	pldw [r0, #4095]
	pldw [r0, #-4095]

	pldw [r0, r0]
	pldw [r1, r0]
	pldw [r14, r0]
	pldw [r0, r1]
	pldw [r0, r14]
	pldw [r0, r0, lsl #2]

	.thumb
	.thumb_func
label2:
	pldw [r0, #0]
	pldw [r14, #0]
	pldw [r1, #0]
	pldw [r0, #4095]
	pldw [r0, #-255]

	pldw [r0, r0]
	pldw [r1, r0]
	pldw [r14, r0]
	pldw [r0, r1]
	pldw [r0, r14]
	pldw [r0, r0, lsl #3]

