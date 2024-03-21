	/* ARMv8.3 Pointer authentication, HINT alias instructions.  */
	.text

	xpaclri
	hint #0x7

	pacia1716
	hint #0x8

	pacib1716
	hint #0xa

	autia1716
	hint #0xc

	autib1716
	hint #0xe

	paciaz
	hint #0x18

	paciasp
	hint #0x19

	pacibz
	hint #0x1a

	pacibsp
	hint #0x1b

	autiaz
	hint #0x1c

	autiasp
	hint #0x1d

	autibz
	hint #0x1e

	autibsp
	hint #0x1f
