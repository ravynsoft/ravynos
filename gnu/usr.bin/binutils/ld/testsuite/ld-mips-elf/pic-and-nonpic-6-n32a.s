	.abicalls
	.global	extf1
	.ent	extf1
extf1:
	jr	$31
	.end	extf1

	.global	extf2
	.ent	extf2
extf2:
	jr	$31
	.end	extf2

	.global	extf3
	.ent	extf3
extf3:
	jr	$31
	.end	extf3

	.global	extf4
	.ent	extf4
extf4:
	jr	$31
	.end	extf4

	.global	extf5
	.ent	extf5
extf5:
	jr	$31
	.end	extf5

	.data
	.global	extd1
	.global	extd2
	.global	extd3
	.global	extd4
	.type	extd1,%object
	.type	extd2,%object
	.type	extd3,%object
	.type	extd4,%object
	.size	extd1,20
	.size	extd2,24
	.size	extd3,28
	.size	extd4,8
extd1:	.space	20
extd2:	.space	24
extd3:	.space	28
extd4:	.space	8
