	.text

stuff:
	.ent stuff
	.set push
	.set noreorder
	.set noat

	# sc/ll instructions are not supported on r5900:
	ll $5, 0($6)
	sc $5, 0($6)

	# scd/lld instructions are not supported on r5900:
	lld $5, 0($6)
	scd $5, 0($6)

	.space	8
	.end stuff
