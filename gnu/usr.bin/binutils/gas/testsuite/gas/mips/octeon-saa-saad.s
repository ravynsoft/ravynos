	.text
foo:
	saa $5,($2)
	saad $6,($4)
	nop

	saa $5,0($2)
	saad $6,0($4)
	nop

	saa $5, foo
	saad $2, foo
	nop

	saa $4, 0x12345678
	saad $4, 0x12345678
	nop

	saa $5, 0x1234($4)
	saad $6, 60($0)
	nop

	saa $5, 0x123456($4)
	saa $6, 0x1234($6)
	nop

	saad $4, 0x5678($5)
	saad $5, 0x567891($5)
	nop	

	saa $4, %lo(foo)($5)
	saad $4, %lo(foo)($5)
	nop
