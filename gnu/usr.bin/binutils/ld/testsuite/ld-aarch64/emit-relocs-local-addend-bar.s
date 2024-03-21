	.global	b
	.section	.rodata
	.align	3
.LC0:
	.string	"bar"
	.align	3
	.type	b, %object
	.size	b, 8
b:
	.xword	.LC0
	.text
	.align	2
	.global	bar
	.type	bar, %function
bar:
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	ret
	.size	bar, .-bar
