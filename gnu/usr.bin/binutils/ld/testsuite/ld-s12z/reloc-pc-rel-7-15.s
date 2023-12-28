
here:
	;; BLE
	.byte 0x2f
	.byte 0x80
	.byte 0x00
	nop


.reloc here+1,R_S12Z_PCREL_7_15, foo

