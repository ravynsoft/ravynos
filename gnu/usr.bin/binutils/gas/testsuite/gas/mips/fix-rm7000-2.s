	.section .text2, "ax", @progbits
	.align 2
test2:
	move	$2,$4
	dmultu	$5,$7
	lb	$4,0($fp)
	dmult	$6,$3
	lbu	$7,0($fp)
	move	$2,$4
	dmultu	$5,$7
	lca	$4,0($fp)
	move	$2,$4
	dmult	$6,$3
	lh	$7,0($fp)
	dmultu	$5,$7
	lhu	$4,0($fp)
	dmult	$6,$3
	ll	$7,0($fp)
	dmultu	$5,$7
	lld	$4,0($fp)
	dmultu	$5,$7
	lw	$4,0($fp)
	dmult	$6,$3
	lwr	$7,0($fp)
	dmultu	$5,$7
