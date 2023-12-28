	.text
	.set nomicromips
	.globl test1
	.type	test1, @function
test1:
	la $3,test2+2
	.set micromips
test2:
	.insn
	.half 0x0c00
	.half 0x0c00
# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
