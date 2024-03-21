# Check 287-only instructions.

	.text
	.arch i286
	.arch .287
	.code16
_8087:
	fnsetpm
	frstpm
	fsetpm
