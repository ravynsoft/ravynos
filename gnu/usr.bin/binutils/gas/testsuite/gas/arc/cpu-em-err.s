;;; Check if .cpu em doesn't have code-density ops.
; { dg-do assemble { target arc*-*-* } }
	.cpu	em
	sub_s r15,r2,r15 ; { dg-error "Error: register must be SP for instruction 'sub_s'" }
