;;; Test if the assembler correctly generates PC-relative relocations
;;; related with branch instructions
	.cpu HS
	.text
	bne	@printf
	blne	@printf
	b	@printf
	bl	@printf
	bl_s	@printf
