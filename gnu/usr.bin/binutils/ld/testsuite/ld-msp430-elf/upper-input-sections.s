.text
	.balign 2
  .section .upper.text,"ax",@progbits
	.global	foo
	.type	foo, @function
foo:
  MOV.W	#42, R8
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
	.section	.upper.bss,"aw",@nobits
	.balign 2
	.global	P
P:
	.zero 4
	.section	.upper.data,"aw",@progbits
	.balign 2
	.global	Q
Q:
	.long 4
	.section	.upper.rodata,"aw",@progbits
	.balign 2
	.global	R
R:
	.word 8
