foo:
	# range check
	sb       $s3,4($t0)
	sw       $s3,8($t0)
	sb       $s3,15($t0)
	break

	sb       $s3,3($t0)
	sw       $s3,8($t0)
	sb       $s3,15($t0)
	break

	# overlap (same word)
	sw       $s3,28($t0)
	sw       $s3,8($t0)
	sb       $s3,31($t0)
	break

	# unaligned
	sb       $s3,5($t0)
	sw       $s3,9($t0)
	sb       $s3,16($t0)
	break

	sb       $s3,4($t0)
	sw       $s3,9($t0)
	sb       $s3,16($t0)
	break
	
	# range check
	sb       $s3,6($t0)
	sh       $s3,8($t0)
	sb       $s3,15($t0)
	break

	sb       $s3,5($t0)
	sh       $s3,8($t0)
	sb       $s3,15($t0)
	break

	# overlap (same hword)
	sh       $s3,30($t0)
	sh       $s3,8($t0)
	sb       $s3,31($t0)
	break

	# unaligned
	sb       $s3,7($t0)
	sh       $s3,9($t0)
	sb       $s3,16($t0)
	break

	sb       $s3,6($t0)
	sh       $s3,9($t0)
	sb       $s3,16($t0)
	break
	
	# range check
	sb       $s3,7($t0)
	sdc1     $f0,8($t0)
	sb       $s3,15($t0)
	break

	sb       $s3,7($t0)
	sdc1     $f0,8($t0)
	sb       $s3,16($t0)
	break

	# overlap (same dword)
	sb       $s3,16($t0)
	sdc1     $f0,8($t0)
	sb       $s3,23($t0)
	break

	sb       $s3,16($t0)
	sdc1     $f0,8($t0)
	sb       $s3,24($t0)
	break

	# unaligned
	sb       $s3,8($t0)
	sdc1     $f0,9($t0)
	sb       $s3,16($t0)
	break

	sb       $s3,-3($t0)
	sdc1     $f0,-2($t0)
	sb       $s3,6($t0)
	break

# Force at least 8 (non-delay-slot) zero bytes,to make 'objdump' print ...
	.align	2
	.space	8
