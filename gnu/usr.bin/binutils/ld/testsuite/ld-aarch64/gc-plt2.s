	.text
	.align  2
	.global bar
	.type   bar, %function
bar:
	b foo

	.align  2
	.type   foo, %function
foo:
	bl hidfn
