
	.type foo, %function
	.set foo, 0x100000

arm:	bl	0x100000
	b	0x100000
	blx	0x100000
	bl	foo

	.syntax unified
	.thumb
thumb:	bl	0x100000
	b	0x100000
	blx	0x100000
	@ bl foo is broken - gas fails to preserve the symbol reference
