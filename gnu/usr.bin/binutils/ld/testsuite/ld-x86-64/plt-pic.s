	.text
	.globl foo
	.type foo,@function
foo:
	call fn1@plt
	jmp fn2@plt
