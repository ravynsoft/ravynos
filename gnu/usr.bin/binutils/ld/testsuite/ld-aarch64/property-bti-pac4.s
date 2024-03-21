	.arch armv8-a
	.file	"t.c"
	.text
	.align	2
	.p2align 3,,7
	.global	f
	.type	f, %function
f:
	add	w0, w0, 1
	ret
	.size	f, .-f
	.section	.text.startup,"ax",@progbits
	.align	2
	.p2align 3,,7
	.global	main
	.type	main, %function
main:
	mov	w0, 6
	ret
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
