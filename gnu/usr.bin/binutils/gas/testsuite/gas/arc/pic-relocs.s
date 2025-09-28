;;; Test if the assembler correctly generates @pcl relocations
	.cpu HS
	.text
	ld	r2,[pcl,@var@gotpc]
	add 	gp,pcl,@var@gotpc

	add 	r2,gp,@var@gotoff
