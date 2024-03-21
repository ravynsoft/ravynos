foo:
	beqz $r0, foo
	bgez $r0, foo
	bgezal $r0, foo
	bgtz $r0, foo
	blez $r0, foo
	bltz $r0, foo
	bltzal $r0, foo
