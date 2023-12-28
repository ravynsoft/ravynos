	# Range check after alignment between adjacent offsets >= 24 ??

foo:
	sb       $s3,0($t0)
	sb       $s3,1($t0)
	sb       $s3,24($t0)
	break
	sb       $s3,0($t0)
	sb       $s3,1($t0)
	sb       $s3,25($t0)
	break
	sb       $s3,1($t0)
	sb       $s3,25($t0)
	sb       $s3,26($t0)
	break
	
	sb       $s3,0($t0)
	sh       $s3,3($t0)
	sb       $s3,26($t0)
	break
	sh       $s3,0($t0)
	sb       $s3,3($t0)
	sb       $s3,26($t0)
	break
	sb       $s3,35($t0)
	sh       $s3,32($t0)
	sb       $s3,9($t0)
	break
	sb       $s3,1($t0)
	sh       $s3,25($t0)
	sb       $s3,27($t0)
	break
	
	sb       $s3,0($t0)
	sw       $s3,7($t0)
	sb       $s3,28($t0)
	break
	sb       $s3,0($t0)
	sb       $s3,7($t0)
	sw       $s3,28($t0)
	break
	sb       $s3,64($t0)
	sw       $s3,59($t0)
	sw       $s3,37($t0)
	break
	sw       $s3,64($t0)
	sb       $s3,61($t0)
	sb       $s3,39($t0)
	break
	sb       $s3,1($t0)
	sw       $s3,25($t0)
	sb       $s3,29($t0)
	break

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
