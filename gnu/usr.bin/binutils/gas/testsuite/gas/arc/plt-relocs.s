;;; Test if the assembler correctly generates PC-relative relocations
;;; related with branch instructions
	.cpu HS
	.text
	bne	@printf@plt
	blne	@printf@plt
	b	@printf@plt
	bl	@printf@plt
	add	r0,pcl,@printf@plt
