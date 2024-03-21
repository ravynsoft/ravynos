.text
	.weak foo
	.global main
main:
	b.ne	foo
	b.eq	foo
	b.cs	foo
	b.cc	foo
	b.gt	foo
	b.ge	foo
	b.lt	foo
	b.le	foo
	b	foo
	bl	foo
	ldr	x0, foo
	adr	x0, foo
	adrp	x0, foo
	add	x0, x0, :lo12:foo
