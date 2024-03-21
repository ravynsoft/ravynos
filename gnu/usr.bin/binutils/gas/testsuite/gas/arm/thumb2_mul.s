	.syntax unified
	.text
	.align	2
	.global	thumb2_mul
	.thumb
	.thumb_func
thumb2_mul:
	# These can use the 16-bit encoding.
	itt eq
	muleq r0, r1, r0
	muleq r0, r0, r1
	# These must use the 32-bit encoding because they involve
	# high registers.
	ittt eq
	muleq r0, r0, r8
	muleq r0, r8, r0
	muleq r8, r0, r8
	# These must use the 32-bit encoding because the source and
	# destination do not match.
	itt eq
	muleq r0, r1, r1
	muleq r0, r1, r2
	# These must use the 32-bit encoding because of the explicit
	# suffix.
	itt eq
	muleq.w r0, r1, r0
	muleq.w r0, r0, r1
	
	
