;;; Check if .cpu em4 has code-density ops.
; { dg-do assemble { target arc*-*-* } }
	.cpu	em4
	sub_s r15,r2,r15
