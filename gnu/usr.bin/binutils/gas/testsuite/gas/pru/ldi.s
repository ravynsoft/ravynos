# Source file used to test the LDI instructions.

	.extern var1
foo:
	# immediate load
	ldi32	r16, 0x12345678
	ldi	r16, 0x1234
	ldi	r16, %pmem(foo)
	ldi32	r16, var1
