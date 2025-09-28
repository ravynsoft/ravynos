;;; Check if .cpu em4_fpuda has code-density and fpuda ops.
; { dg-do assemble { target arc*-*-* } }
	.cpu	em4_fpuda
	sub_s 	r15,r2,r15	; code-density op
	dmulh11	r1,r2,r3	; fpuda op
	fadd	r1,r2,r3	; { dg-error "Error: opcode 'fadd' not supported for target em4_fpuda" }
