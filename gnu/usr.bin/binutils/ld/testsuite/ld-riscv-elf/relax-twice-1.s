	.file	"<artificial>"
	.option pic
	.text
	.globl	foobar_new
	.weak	foobar_new
	.type	foobar_new, @function
foobar_new:
	jr ra
	.size	foobar_new, .-foobar_new
	.symver	foobar_new, foobar@@New

	.section	.note.GNU-stack,"",@progbits
