	.text

	# Call the IFUNC `foo` which is defined in the other modules.
	.globl	foo
	.type	foo, %function

	.globl	main
	.type	main, @function
main:
.L1:
	auipc	x1, %got_pcrel_hi (foo)
	addi	x1, x1, %pcrel_lo (.L1)

.L2:
	auipc	x2, %pcrel_hi (foo_addr)
	addi	x2, x2, %pcrel_lo (.L2)

	call	foo
	call	foo@plt

	ret
	.size	main, .-main

	.data
foo_addr:
	.quad	foo
