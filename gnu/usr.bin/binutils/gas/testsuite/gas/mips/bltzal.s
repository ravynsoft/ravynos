	.set	reorder
test:
	bltzal	$31, test
	bgezal	$31, test
	.set	mips2
	bltzall	$31, test
	bgezall	$31, test
	.set	micromips
	bltzals	$31, test
	bgezals	$31, test
