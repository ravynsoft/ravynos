	.text

	# Call the IFUNC `foo` which is defined in the other modules.
	.globl	foo
	.type	foo, %function

	.globl	main
	.type	main, @function
main:
.L1:
	auipc	x1, %pcrel_hi (foo)
	addi	x1, x1, %pcrel_lo (.L1)
	ret
	.size	main, .-main
