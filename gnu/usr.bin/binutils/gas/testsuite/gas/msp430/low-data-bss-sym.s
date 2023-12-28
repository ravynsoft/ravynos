	.file	"main.c"
.text
	.global	a
	.section	.lower.data,"aw"
	.balign 2
	.type	a, @object
	.size	a, 2
a:
	.short	42
	.global	b
	.section	.lower.bss,"aw",@nobits
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
