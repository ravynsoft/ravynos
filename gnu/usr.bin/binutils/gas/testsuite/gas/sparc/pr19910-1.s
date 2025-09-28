	# U suffix should not trigger an error.
	sethi %hi(0x4000U), %g1

	# U suffix should not prevent evaluation of the expression.
        or %g0, %lo(0x400U + 0x40U), %g1

	# U suffix should not confuse multiple layers of parentheses.
        or %g0, %lo((0x4000U + 0x4U)), %g1
