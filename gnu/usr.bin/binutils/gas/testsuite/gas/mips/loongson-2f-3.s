# Test the work around of the Jump instruction Issue of Loongson2F
	.text
	.set noreorder

	j	$30	# j with register
	 nop

	jr	$31	# jr
	 nop

	jalr	$30	# jalr
	 nop

	.set	noat
	jr	$1	# jr with at register and .set annotation
	 nop
	.set	at

	j	external_label	# j with label
	 nop

# align section end to 16-byte boundary for easier testing on multiple targets
	.p2align 4
