;;; This is really a test of the disassembler rather than
;;; the assembler.
	nop
        DC.L 0x03A51004
        DC.B 0x06
        nop

        nop
        DC.L 0x03651201
        nop
	dc.w 0xEC44
	dc.w 0xEC7C
	dc.w 0xED5D
	dc.w 0xED7D

