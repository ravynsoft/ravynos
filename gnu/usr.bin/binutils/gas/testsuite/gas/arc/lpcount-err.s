;; LP_COUNT register cannot be used with multi-cycle instructions such as:
;; load, lr, multiply and divide.
; { dg-do assemble { target arc*-*-* } }

	.cpu HS

	mpy	lp_count,r0,r1	; { dg-error "Error: LP_COUNT register cannot be used as destination register." }
	ld	lp_count,[r2,1]	; { dg-error "Error: LP_COUNT register cannot be used as destination register." }
	div	lp_count,r12,r1	; { dg-error "Error: LP_COUNT register cannot be used as destination register." }
