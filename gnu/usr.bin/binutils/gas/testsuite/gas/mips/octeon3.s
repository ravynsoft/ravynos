	.text
	.set noreorder
	.set noat

foo:
	mtm0 $15,$12
	mtm0 $13,$4

	mtm1 $4,$3
	mtm1 $7,$1

	mtm2 $1,$2
	mtm1 $4,$3

	mtp0 $5,$2
	mtp0 $6,$4

	mtp1 $4,$3
	mtp1 $7,$1

	mtp2 $1,$2
	mtp1 $4,$3
