	.section .text2, "ax", @progbits
	.align 2
test1:
	move	$2,$4
	dmult	$6,$3
	ld	$7,0($fp)
	ld	$4,0($fp)
	move	$4,$7
	dmult	$2,$7
	ld	$4,0($fp)
