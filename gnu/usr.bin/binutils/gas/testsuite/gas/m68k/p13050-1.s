# The following addressing mode is forbidden even on isab or higher (PR13050)
	move.b	(2,%a0,%d0.l),1(%a1)
