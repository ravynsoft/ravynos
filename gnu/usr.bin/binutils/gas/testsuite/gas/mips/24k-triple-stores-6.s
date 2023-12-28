	# Store macros

foo:
	.ifndef r6
	usw      $ra,80($sp)
	usw      $s3,88($sp)
	usw      $s8,96($sp)
	break

	ush      $ra,80($sp)
	ush      $s3,88($sp)
	ush      $s8,96($sp)
	break
	.endif

	# swc1 macro
	s.s      $f0,80($sp)
	s.s      $f2,88($sp)
	s.s      $f4,96($sp)
	break

        # sdc1 macro
	s.d      $f0,80($sp)
	s.d      $f2,88($sp)
	s.d      $f4,96($sp)
break

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
