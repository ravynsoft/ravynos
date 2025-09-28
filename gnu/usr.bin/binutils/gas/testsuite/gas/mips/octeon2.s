	.text
	.set noreorder
	.set noat

foo:
	qmac.00	$3,$16
	qmac.01	$13,$10
	qmac.02 $2,$13
	qmac.03	$16,$23

	qmacs.00 $2,$10
	qmacs.01 $30,$10
	qmacs.02 $12,$13
	qmacs.03 $6,$6

	lbx	$15,$9($3)
	lbux	$5,$28($15)
	lhx	$17,$25($6)
	lhux	$31,$21
	lwx	$21,$29($5)
	lwux	$18,$5($6)
	ldx	$22,$29($23)

	laa	$2,($17),$13
	laad	$7,($14),$16
	law	$26,($8),$5
	lawd	$20,($15),$22
	lai	$7,($3)
	laid	$31,($11)
	lad	$25,($31)
	ladd	$30,($4)
	las	$13,($8)
	lasd	$27,($30)
	lac	$13,($28)
	lacd	$7,($8)

	zcb	($19)
	zcbt	($17)
