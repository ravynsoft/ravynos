.text
	.balign 2
	.global	foo
	.type	foo, @function
foo:
  MOV.W	#lo (P), R8
	RETA
	.size	foo, .-foo

	.balign 2
	.global	main
	.type	main, @function
main:
	CALLA	#foo
.L4:
	BRA	#.L4
	.size	main, .-main
	.section	.bss,"aw",@nobits
	.balign 2
	.global	P
P:
	.zero	4
