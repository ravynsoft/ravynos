	.section bar
	.nop
	.attach_to_group foo.group
	
	.section foo, "G", %note , foo.group
	.word 0

	.section bar
	.nop
