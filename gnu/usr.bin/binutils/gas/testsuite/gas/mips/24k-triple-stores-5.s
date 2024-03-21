# Mix byte/half/word sizes with arbitrary base register.

foo:
	# safe
	sh      $2,7($8)
	sb      $3,0($8)
	sw      $4,1($8)
	break

	# nop
	sh      $2,22($8)
	sb      $3,15($8)
	sw      $4,24($8)
	break

	# safe
	sh      $2,0($8)
	sb      $3,9($8)
	sw      $4,2($8)
	break

	# nop
	sh      $2,6($8)
	sb      $3,16($8)
	sw      $4,12($8)
	break

	# safe
	sh      $2,10($8)
	sb      $3,15($8)
	sw      $4,4($8)
	break

	# nop
	sh      $2,10($8)
	sb      $3,16($8)
	sw      $4,4($8)
	break

# Force at least 8 (non-delay-slot) zero bytes,to make 'objdump' print ...
	.align	2
	.space	8
