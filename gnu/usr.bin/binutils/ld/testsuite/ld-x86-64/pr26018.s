	.global _start, foo
	.type foo, %function
	.text
_start:
	call foo@PLT
foo:
	ret
