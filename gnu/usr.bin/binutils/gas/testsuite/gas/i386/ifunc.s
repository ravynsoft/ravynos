	.global foo
	.type foo, @function
foo:
	jmp ifunc@PLT
	.type ifunc, @gnu_indirect_function
ifunc:
	ret
	.global bar
	.type bar, @function
bar:
	jmp normal@PLT
	.type normal, @function
normal:
	ret
