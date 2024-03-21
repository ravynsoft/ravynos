; { dg-do assemble { target arc*-*-* } }
	.extCondCode    cctst, -12 ; { dg-error "Error: extCondCode's second argument cannot be a negative number -12" }
