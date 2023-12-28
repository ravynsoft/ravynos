# Check for range

foo:
	sb      $2,0($sp)
	sb      $3,10($sp)
	sb      $4,31($sp)
	break

	sh      $2,0($sp)
	sh      $3,-16($sp)
	sh      $4,-32($sp)
	break

	sw      $2,0($sp)
	sw      $3,-8($sp)
	sw      $4,8($sp)
	break

	.ifndef r6
	swr      $2,0($sp)
	swr      $3,-16($sp)
	swr      $4,16($sp)
	break

	swl      $2,0($sp)
	swl      $3,8($sp)
	swl      $4,16($sp)
	swl      $5,24($sp)
	swl      $6,0($sp)
	.endif

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
