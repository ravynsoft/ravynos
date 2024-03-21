# Verify interAptiv MR2 MIPS16e2 COPYW/UCOPYW ASMACRO instruction errors.

	.text
foo:
	copyw	$2, $2, 0, 0
	copyw	$2, $2, 0, 5
	copyw	$2, $2, -16, 1
	copyw	$2, $2, -15, 1
	copyw	$2, $2, -1, 1
	copyw	$2, $2, 1, 1
	copyw	$2, $2, 15, 1
	copyw	$2, $2, 512, 1
	copyw	$2, $1, 0, 1
	copyw	$2, $8, 0, 1
	copyw	$1, $2, 0, 1
	copyw	$8, $2, 0, 1
	copyw	$2, $2, 1, 0
	copyw	$8, $1, 1, 0
	ucopyw	$2, $2, 0, 0
	ucopyw	$2, $2, 0, 5
	ucopyw	$2, $2, -16, 1
	ucopyw	$2, $2, -15, 1
	ucopyw	$2, $2, -1, 1
	ucopyw	$2, $2, 1, 1
	ucopyw	$2, $2, 15, 1
	ucopyw	$2, $2, 512, 1
	ucopyw	$2, $1, 0, 1
	ucopyw	$2, $8, 0, 1
	ucopyw	$1, $2, 0, 1
	ucopyw	$8, $2, 0, 1
	ucopyw	$2, $2, 1, 0
	ucopyw	$8, $1, 1, 0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
