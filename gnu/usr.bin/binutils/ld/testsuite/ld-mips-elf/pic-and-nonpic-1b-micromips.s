	.abicalls
	.option	pic0
	.global	__start
	.set	micromips
	.ent	__start
__start:
	jal	f1
	jal	f2
	jal	f3
	.end	__start
