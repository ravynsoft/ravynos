	.file	"main.c"
.text
	.global	a
	.section	.either.data,"aw"
	.balign 2
	.type	a, @object
	.size	a, 2
a:
	.short	42
	.global	b
	.section	.either.bss,"aw",@nobits
	.balign 2
	.type	b, @object
	.size	b, 2
b:
	.zero	2
.text
	.balign 2
	.global	main
	.type	main, @function
main:
.L2:
	BR	#.L2
	.size	main, .-main
