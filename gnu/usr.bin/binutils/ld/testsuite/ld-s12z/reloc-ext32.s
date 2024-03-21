	
here:
	nop
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	nop


.reloc here+1,R_S12Z_EXT32, foobar

