; Test error messages where targets are out of range.

; { dg-do assemble }

	.text
L1:
	.rept	32768
	nop
	.endr
	brr	tr,L1
L2:
	.rept	32769
	nop
	.endr
	brr	tr,L2 ; { dg-error "out of range" "out of range brr" }
