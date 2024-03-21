	sll.ob	$f2,$f4,4
	sll.ob	$f2,$f4,$f6[1]
	sll.ob	$f2,$f4,$f6

	srl.ob	$f2,$f4,4
	srl.ob	$f2,$f4,$f6[1]
	srl.ob	$f2,$f4,$f6

	rzu.ob	$f2,4
	rzu.ob	$f2,$f6[1]
	rzu.ob	$f2,$f6

	add.ob	$f2,$f4,$f6
	add.ob	$v2,$f4,$f6
	add.ob	$f2,$v4,$f6
	add.ob	$f2,$f4,$v6
	add.ob	$v2,$v4,$v6

	add.ob	$f2,$f4,$f6[1]
	add.ob	$v2,$f4,$f6[1]
	add.ob	$f2,$v4,$f6[1]
	add.ob	$f2,$f4,$v6[1]
	add.ob	$v2,$v4,$v6[1]

	add.ob	$f2,$f4,$f6[foo]
	add.ob	$f2,$f4,$f6[1}
