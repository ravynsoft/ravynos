
;;; Both the BNE instructions should fail
;;; because the destination is out of range.
	bne .label
	.fill 0x3FFD, 1, 0  	; 0x3FFF minus 3 (the length of the BNE insn)
.label:
	.fill 0x4001, 1, 0
	bne .label
