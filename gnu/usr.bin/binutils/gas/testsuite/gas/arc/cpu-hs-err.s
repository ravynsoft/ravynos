;;; Check if .cpu hs has code-density
; { dg-do assemble { target arc*-*-* } }
	.cpu	hs
	sub_s 	r15,r2,r15	; code-density op
	dmulh11	r1,r2,r3	; { dg-error "Error: opcode 'dmulh11' not supported for target hs" }
