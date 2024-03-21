	.text
	bnd call	0x100040
	bnd jmp		0x100040

	bnd data16 rex.w call	0x100040
	bnd data16 rex.w jmp	0x100040

foo1:
	bnd jmp		foo1
	bnd jb		foo1
	bnd call	foo1
	bnd jmp		foo2
	bnd jb		foo2
	bnd call	foo2
foo2:
	bnd jmp		foo
	bnd jb		foo
	bnd call 	foo
	bnd jmp		foo@PLT
	bnd jb		foo@PLT
	bnd call	foo@plt
