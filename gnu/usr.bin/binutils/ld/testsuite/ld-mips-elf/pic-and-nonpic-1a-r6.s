	.abicalls
	.global	f1
	.global	f2
	.global	f3
	.ent	f1
f1:
	.set	noreorder
	.cpload	$25
	.set	reorder
	.option	pic0
	jal	f3
	.option	pic2
	jr	$31
	.end	f1

	.ent	f2
f2:
	.set	noreorder
	.cpload	$25
	.set	reorder
	jr	$31
	.end	f2

	.ent	f3
f3:
	sll	$2,16
	addu	$2,$2,$3
	.end	f3
