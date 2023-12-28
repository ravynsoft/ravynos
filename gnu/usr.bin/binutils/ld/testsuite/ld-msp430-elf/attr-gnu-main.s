.text
	.balign 2
	.global	main
	.type	main, @function
main:
.L2:
	BRA	#.L2
	.size	main, .-main
