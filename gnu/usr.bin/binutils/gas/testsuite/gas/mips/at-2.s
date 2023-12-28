	.text
foo:
	.set	at=$0
	lw	$1, 0x8000($1)
	lw	$26, 0x8000($26)
	lw	$27, 0x8000($27)
	.set	at=$1
	lw	$1, 0x8000($1)
	lw	$26, 0x8000($26)
	lw	$27, 0x8000($27)
	.set	at=$26
	lw	$1, 0x8000($1)
	lw	$26, 0x8000($26)
	lw	$27, 0x8000($27)
	.set	at=$27
	lw	$1, 0x8000($1)
	lw	$26, 0x8000($26)
	lw	$27, 0x8000($27)

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
