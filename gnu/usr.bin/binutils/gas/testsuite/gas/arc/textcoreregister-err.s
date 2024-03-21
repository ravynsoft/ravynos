; { dg-do assemble { target arc*-*-* } }
	.extCoreRegister r32c, -32, r|w, can_shortcut ; { dg-error "Error: extCoreRegister's second argument cannot be a negative number -32" }
