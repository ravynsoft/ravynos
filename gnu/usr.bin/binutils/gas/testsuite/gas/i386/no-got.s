	.global foo
	.type foo, @function
foo:
	jmp bar@PLT
	call bar@PLT
