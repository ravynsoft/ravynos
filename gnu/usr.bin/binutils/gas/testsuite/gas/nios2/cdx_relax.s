# Test relaxation of beqz.n, bnez.n, and br.n instructions to
# equivalent 32-bit instructions when the branch target is out of range.

	.text

# These branches are within range.
label0:
	bnez.n r2, label1
	beqz.n r3, label1
	br.n label1

# These branches have an out-of-range positive offset.
label1:
	bnez.n r2, label2
	beqz.n r3, label2
	br.n label2

	.rept 300
	nop
	.endr

# These branches have an out-of-range negative offset.
label2:
	bnez.n r2, label1
	beqz.n r3, label1
	br.n label1

