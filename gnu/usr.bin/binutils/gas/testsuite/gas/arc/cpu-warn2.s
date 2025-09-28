; Test warnings when multiple .cpu pseudo-ops are defined.
; { dg-do assemble }
	.cpu EM
	.cpu HS			;{ dg-error "Error: Multiple .cpu directives found" }
