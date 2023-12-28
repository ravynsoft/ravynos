# { dg-do assemble { target xtensa*-*-* } }
	.text
	.global foo
	beqz	a2, 1f@pcrel	# { dg-error "relocation not allowed" "" }
1:	movi	a2, foo@pcrel	# { dg-error "relocation not allowed" "" }
foo:	.short	foo@pcrel	# { dg-error "relocations do not fit" "" }
