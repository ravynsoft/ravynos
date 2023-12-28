	.text
foo:
	sb $2,0($8)
	.data
	.word 1
	.text
	sb $3,8($8)
	.data
	.word 1
	.text
	sb $4,16($8)
