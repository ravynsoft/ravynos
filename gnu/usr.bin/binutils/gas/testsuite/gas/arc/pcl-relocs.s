;;; Test if the assembler correctly generates @pcl relocations
	.cpu HS
	.text
	add	r0,pcl,@var@pcl
	ldd	r6,[pcl,@var@pcl]
	ld	r3,[pcl,@var@pcl]
