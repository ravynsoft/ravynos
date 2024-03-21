	.text
	.irp opc, 18, 19, 1a, 1b, 1c, 1d, 1e, 1f
_0f\opc:
	.irp pfx, , 66, f3, f2
	.irp mod, 3, 0
	.irp reg, 0, 1, 2, 3, 4, 5, 6, 7
	.irp rm, 0, 1, 2, 3, 4, 5, 6, 7
	.if !\mod && \rm
	.exitm
	.endif
	.ifnb \pfx
	.byte 0x\pfx
	.endif
	.byte 0xf, 0x\opc, (\mod << 6) | (\reg << 3) | \rm
	.endr
	.endr
	.endr
	.endr
	.endr
