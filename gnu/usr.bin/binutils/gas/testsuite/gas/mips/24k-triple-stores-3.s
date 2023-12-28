# Assume to be on the same line (within 32bytes)
# Check for individual different double words

foo:
	# safe
	sb      $2,11($sp)
	sb      $3,11($sp)
	sb      $4,4($sp)
	break

	# safe
	sb      $2,0($sp)
	sb      $3,11($sp)
	sb      $4,5($sp)
	break
	
	# edge case
	sb      $2,7($sp)
	sb      $3,11($sp)
	sb      $4,16($sp)
	break

	# edge case (unaligned)
	sb      $2,0($8)
	sb      $3,8($8)
	sb      $4,9($8)	
	break

	sh      $2,0($sp)
	sh      $3,-31($sp)
	sh      $4,-30($sp)
	break

	# edge case
	sh      $2,6($sp)
	sh      $3,8($sp)
	sh      $4,16($sp)
	break

	# edge case (unaligned)
	sh      $2,1($8)
	sh      $3,3($8)
	sh      $4,11($8)	
	break

	sw      $2,8($sp)
	sw      $3,-8($sp)
	sw      $4,8($sp)
	break

	# edge case
	sw      $2,4($sp)
	sw      $3,8($sp)
	sw      $4,16($sp)
	break

	# edge case (unaligned)
	sw      $2,3($8)
	sw      $3,7($8)
	sw      $4,15($8)	
	break

	.ifndef r6
	swl      $2,4($sp)
	swl      $3,10($sp)
	swl      $4,17($sp)
	break

	# edge case
	swl      $2,7($sp)
	swl      $3,12($sp)
	swl      $4,16($sp)
	break

	# edge case
	swl      $2,0($sp)
	swl      $3,12($sp)
	swl      $4,23($sp)
	break

	# edge case (unaligned)
	swl      $2,3($8)
	swl      $3,8($8)
	swl      $4,12($8)
	break

	# mix swl & swr
	swl      $2,0($sp)
	swl      $3,12($sp)
	swr      $4,23($sp)
	break

	# mix swl & swr
	swl      $2,5($8)
	swl      $3,17($8)
	swr      $4,28($8)
	break		
	.endif

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
