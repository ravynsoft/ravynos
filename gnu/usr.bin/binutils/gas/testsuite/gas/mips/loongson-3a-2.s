	.text
	.set noreorder

	gsle		$11,$12
	gsgt		$13,$14

	gslble		$2,$3,$4
	gslbgt		$5,$6,$7
	gslhle		$8,$9,$10
	gslhgt		$11,$12,$13
	gslwle		$14,$15,$16
	gslwgt		$17,$18,$19
	gsldle		$20,$21,$22
	gsldgt		$23,$24,$25
	gssble		$2,$3,$4
	gssbgt		$5,$6,$7
	gsshle		$8,$9,$10
	gsshgt		$11,$12,$13
	gsswle		$14,$15,$16
	gsswgt		$17,$18,$19
	gssdle		$20,$21,$22
	gssdgt		$23,$24,$25

	gslwlec1	$f0,$2,$3
	gslwgtc1	$f1,$4,$5
	gsldlec1	$f2,$6,$7
	gsldgtc1	$f3,$8,$9
	gsswlec1	$f4,$10,$11
	gsswgtc1	$f5,$12,$13
	gssdlec1	$f6,$14,$15
	gssdgtc1	$f7,$16,$17

	gslwlc1		$f8,0($18)
	gslwrc1		$f9,1($19)
	gsldlc1		$f10,2($20)
	gsldrc1		$f11,3($21)
	gsswlc1		$f12,4($22)
	gsswrc1		$f13,5($23)
	gssdlc1		$f14,6($24)
	gssdrc1		$f15,7($25)

	gslbx		$2,0($3,$4)
	gslhx		$5,-1($6,$7)
	gslwx		$8,-2($9,$10)
	gsldx		$11,-3($12,$13)
	gssbx		$14,-4($15,$16)
	gsshx		$17,-5($18,$19)
	gsswx		$20,-6($21,$22)
	gssdx		$23,-7($24,$25)

	gslwxc1		$f16,127($2,$3)
	gsldxc1		$f17,-128($4,$5)
	gsswxc1		$f18,127($6,$7)
	gssdxc1		$f19,-128($8,$9)

	gslq		$10,$11,4080($12)
	gssq		$13,$14,-4096($15)
	gslqc1		$f20,$f21,4080($16)
	gssqc1		$f22,$f23,-4096($17)
