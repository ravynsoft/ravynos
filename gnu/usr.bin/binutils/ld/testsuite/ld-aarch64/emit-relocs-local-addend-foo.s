	.cpu generic+fp+simd
	.global	a
	.section	.rodata
	.align	3
.LC0:
	.string	"foo"
	.align	3
	.type	a, %object
	.size	a, 8
a:
	.xword	.LC0
	.text
	.align	2
	.global	foo
	.type	foo, %function
foo:
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	ret
	.size	foo, .-foo
