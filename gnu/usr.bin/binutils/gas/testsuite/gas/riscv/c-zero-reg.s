	.option arch, +c
	# These are hints.
	c.li x0, 1
	c.lui x0, 2
	c.slli x0, 3
	c.mv x0, x1
	c.add x0, x1
	# Don't let these compress to hints.
	li x0, 5
	lui x0, 6
	slli x0, x0, 7
	mv x0, x1
	add x0, x0, x1
# RV128 support not implemented yet.
#	slli x0, x0, 64
