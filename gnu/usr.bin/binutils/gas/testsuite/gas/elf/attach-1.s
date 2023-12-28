	.text
	.nop
	
	.section foo, "G", %progbits , foo.group
	.word 0

	.text
	/* This is the intended use of the .attach_to_group directive.
	   It attaches a previously defined section (.text) to a
	   previously defined group (foo.group).  */
	.attach_to_group foo.group
