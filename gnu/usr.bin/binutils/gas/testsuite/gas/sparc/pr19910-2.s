	# Mistyping V instead of U should trigger an error message
	sethi %hi(0x4000V), %g1

	# A different error can be expected when there are multiple layers of parentheses.
        or %g0, %lo((0x4000V + 0x4U)), %g1
