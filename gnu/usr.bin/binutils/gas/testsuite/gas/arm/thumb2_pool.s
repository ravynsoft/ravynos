	.text
	.thumb
	.syntax unified
	.thumb_func
thumb2_ldr:
	ldr	r6, =0x12345678
	ldr.n	r1, =0x12345678
	ldr.w	r6, =0x12345678
	ldr	r9, =0x12345678
	nop
	ldr.w	r5, =0x12345678
	ldr	r1, =0x12345678
	.pool

	# test symbol literal support.
	.macro ldrs const
	.irp regindex, 1, 4, 9, 12, 13
		ldr r\regindex, \const
	.endr
	.endm

	ldrs	"=ext_symbol"
	ldrs	"=ext_symbol + 0x1000"
	.pool
