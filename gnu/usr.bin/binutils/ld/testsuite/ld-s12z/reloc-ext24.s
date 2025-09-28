	
here:
	;; jmp foobar
	.byte 0xba
	.byte 0x00
	.byte 0x00
	.byte 0x00
	nop


.reloc here+1,R_S12Z_EXT24, foobar

