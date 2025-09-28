; Test error messages when targets are out of range

; { dg-do assemble }

	.text
	brr	tr,L1
	.rept	32766
	nop
	.endr
L1:
	brr	tr,L2 ; { dg-error "out of range" "out of range brr" }
	.rept	32767
	nop
	.endr
L2:
	.end
