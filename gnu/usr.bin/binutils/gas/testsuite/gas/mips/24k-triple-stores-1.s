# integer stores

foo:
	sb      $2,0($sp)
	sb      $3,8($sp)
	sb      $4,16($sp)
	sb      $5,24($sp)
	sb      $6,32($sp)

	sh      $2,0($sp)
	sh      $3,8($sp)
	sh      $4,16($sp)
	sh      $5,24($sp)
	sh      $6,32($sp)

	sw      $2,0($sp)
	sw      $3,8($sp)
	sw      $4,16($sp)
	sw      $5,24($sp)
	sw      $6,32($sp)

	.ifndef r6
	swr     $2,0($sp)
	swr     $3,8($sp)
	swr     $4,16($sp)
	swr     $5,24($sp)
	swr     $6,32($sp)

	swl     $2,0($sp)
	swl     $3,8($sp)
	swl     $4,16($sp)
	swl     $5,24($sp)
	swl     $6,32($sp)
	.endif

	sc      $2,0($sp)
	sc      $3,8($sp)
	sc      $4,16($sp)
	sc      $5,24($sp)
	sc      $6,32($sp)

# floating point stores

	swc1    $2,0($sp)
	swc1    $3,8($sp)
	swc1    $4,16($sp)
	swc1    $5,24($sp)
	swc1    $6,32($sp)

	swc2    $2,0($sp)
	swc2    $3,8($sp)
	swc2    $4,16($sp)
	swc2    $5,24($sp)
	swc2    $6,32($sp)

	sdc1    $2,0($sp)
	sdc1    $3,8($sp)
	sdc1    $4,16($sp)
	sdc1    $5,24($sp)
	sdc1    $6,32($sp)

	sdc2    $2,0($sp)
	sdc2    $3,8($sp)
	sdc2    $4,16($sp)
	sdc2    $5,24($sp)
	sdc2    $6,32($sp)

	.ifndef r6
	swxc1   $f0,$9($8)
	swxc1   $f1,$10($8)
	swxc1   $f2,$11($8)
	swxc1   $f3,$12($8)
	swxc1   $f4,$13($8)

	sdxc1   $f0,$9($8)
	sdxc1   $f2,$10($8)
	sdxc1   $f4,$11($8)
	sdxc1   $f6,$12($8)
	sdxc1   $f8,$13($8)

	suxc1   $f0,$9($8)
	suxc1   $f2,$10($8)
	suxc1   $f4,$11($8)
	suxc1   $f6,$12($8)
	suxc1   $f8,$13($8)
	.endif

# Force at least 8 (non-delay-slot) zero bytes,to make 'objdump' print ...
	.align	2
	.space	8
