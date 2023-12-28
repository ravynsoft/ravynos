# Verify the interAptiv MR2 MIPS16e2 COPYW/UCOPYW ASMACRO instructions.

	.text
foo:
	copyw	$2, $3, 0, 1
	copyw	$3, $4, 16, 2
	copyw	$4, $5, 32, 3
	copyw	$5, $6, 64, 4
	copyw	$6, $7, 128, 1
	copyw	$7, $16, 256, 2
	copyw	$16, $17, 384, 3
	copyw	$17, $2, 448, 4
	copyw	$2, $3, 480, 1
	copyw	$3, $4, 496, 2
	copyw	$4, $5, 160, 3
	copyw	$5, $6, 336, 4
	ucopyw	$6, $7, 0, 1
	ucopyw	$7, $16, 16, 2
	ucopyw	$16, $17, 32, 3
	ucopyw	$17, $2, 64, 4
	ucopyw	$2, $3, 128, 1
	ucopyw	$3, $4, 256, 2
	ucopyw	$4, $5, 384, 3
	ucopyw	$5, $6, 448, 4
	ucopyw	$6, $7, 480, 1
	ucopyw	$7, $16, 496, 2
	ucopyw	$16, $17, 160, 3
	ucopyw	$17, $2, 336, 4

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
