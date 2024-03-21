# Source file to test immediates used with the DADDI instruction.

	.set	noreorder
	.set	noat

	.text
text_label:
	.ifndef r6
	daddi	$3, $2, 511
	daddi	$5, $4, -512

	# 10 bits accepted for microMIPS code.
	.ifdef	micromips
	.set	at
	.endif
	daddi	$7, $6, 512
	daddi	$9, $8, -513
	daddi	$11, $10, 32767
	daddi	$13, $12, -32768
	.endif

	# 16 bits accepted for standard MIPS code.
	.ifndef	micromips
	.set	at
	.endif
	dadd	$15, $14, 32768
	dadd	$17, $16, -32769
	dadd	$19, $18, 33280
	dadd	$21, $20, -33281

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
