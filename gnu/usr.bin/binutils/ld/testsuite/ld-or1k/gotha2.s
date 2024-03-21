	.section	.text
	.align 4
	.global	test
	.type	test, @function
test:
	l.addi	r1, r1, -8
	l.sw	0(r1), r16
	l.sw	4(r1), r9

	l.jal	8
	 l.movhi	r16, gotpchi(_GLOBAL_OFFSET_TABLE_-4)
	l.ori	r16, r16, gotpclo(_GLOBAL_OFFSET_TABLE_+0)
	l.add	r16, r16, r9

	l.movhi	r17, gotha(i)
	l.add	r17, r17, r16
	l.lwz	r17, got(i)(r17)

	l.lwz	r9, 4(r1)
	l.lwz	r16, 0(r1)
	l.jr	r9
	 l.addi	r1, r1, 8
