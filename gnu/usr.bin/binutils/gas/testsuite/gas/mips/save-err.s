
	save	$3,100		# error
	save	$4		# error
	save	$4,100,200	# error
	save	$4,foo		# error
	save	$4,0		# OK
	save	$4,1		# error
	save	$4,7		# error
	save	$4,8		# OK
	save	$4,12		# error
	save	$4,2048		# OK
	save	$4,2052		# error
	save	$4,0,$7		# error
	save	$4,$6,0		# error
	save	0,$5,$7		# error
	save	$16,$18,0	# OK
	save	$16,$18,$19,0	# OK
	save	$16,$18,$20,0	# error
