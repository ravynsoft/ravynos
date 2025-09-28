	# Range check for safe case after alignment its range >= 32.

foo:
	sb       $s3,10($t0)
	sh       $s3,1($t0)
	sb       $s3,32($t0)
	break
	sb       $s3,10($t0)
	sb       $s3,1($t0)
	sh       $s3,32($t0)
	break
	sb       $s3,33($t0)
	sh       $s3,55($t0)
	sb       $s3,64($t0)
	break
	sb       $s3,33($t0)
	sb       $s3,55($t0)
	sh       $s3,64($t0)
	break
	
	sb       $s3,12($t0)
	sw       $s3,1($t0)
	sb       $s3,32($t0)
	break
	sb       $s3,12($t0)
	sb       $s3,1($t0)
	sw       $s3,32($t0)
	break
	sb       $s3,35($t0)
	sw       $s3,55($t0)
	sb       $s3,64($t0)
	break
	sb       $s3,35($t0)
	sb       $s3,55($t0)
	sw       $s3,64($t0)
	break
	
	sb       $s3,16($t0)
	sdc1     $f0,1($t0)
	sb       $s3,32($t0)
	break
	sb       $s3,16($t0)
	sb       $s3,1($t0)
	sdc1     $f0,32($t0)
	break
	sb       $s3,39($t0)
	sdc1     $f0,55($t0)
	sb       $s3,64($t0)
	break
	sb       $s3,39($t0)
	sb       $s3,55($t0)
	sdc1     $f0,64($t0)
	break

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
