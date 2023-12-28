	.ent	test
test:
	.frame	$sp,32,$31
	.cprestore 16
	jal	local
	jal	local+12
	jal	global
	jal	global+12
	.end	test

local:
	nop
	nop
	nop
	nop
