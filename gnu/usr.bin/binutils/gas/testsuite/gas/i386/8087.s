# Check 8087-only instructions.

	.text
	.arch i8086
	.arch .8087
	.code16
_8087:
	fdisi
	feni
	fndisi
	fneni
