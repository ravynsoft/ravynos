# Check for range (sc)

foo:
	sc      $2,32($sp)
	sc      $3,8($sp)
	sc      $4,-8($sp)
	sc      $5,0($sp)
	sc      $6,32($sp)

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
