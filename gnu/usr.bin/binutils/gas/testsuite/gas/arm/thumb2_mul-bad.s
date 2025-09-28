	.syntax unified
	.text
	.align	2
	.global	thumb2_mul
	.thumb
	.thumb_func
thumb2_mul:
	itttt eq
	# Cannot use 16-bit encoding because of use of high register.
	muleq.n r0, r0, r8
	# Cannot use 16-bit encoding because source does not match destination.
	muleq.n r0, r1, r1
	muleq.n r0, r1, r2
	# There is no conditional "muls".
	mulseq r0, r0, r1
	# There is no 32-bit "muls".
	muls.w r0, r0, r1
	# Cannot use high registers with "muls".
	muls r0, r0, r8
	muls r0, r8, r0
