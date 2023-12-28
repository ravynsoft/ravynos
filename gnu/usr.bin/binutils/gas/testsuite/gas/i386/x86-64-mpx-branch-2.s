	.text
	call	0x100040
	jmp	0x100040

	data16 rex.w call	0x100040
	data16 rex.w jmp	0x100040

foo1:
	jmp	foo1
	jb	foo1
	call	foo1
	jmp	foo2
	jb	foo2
	call	foo2
foo2:
	jmp	foo
	jb	foo
	call 	foo
	jmp	foo@PLT
	jb	foo@PLT
	call	foo@plt
