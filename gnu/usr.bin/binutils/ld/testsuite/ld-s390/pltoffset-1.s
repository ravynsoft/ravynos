	.file	"hello.c"
.text
	.align	8
.globl main
	.type	main, @function
main:
	brasl	%r5,foo@PLT
	br	%r4
	.size	main, .-main

.globl foo
foo:	.long	123
