; Test error messages for new syntax of FCMP/FCMPE

; { dg-do assemble }
; { dg-options "-mtune=gr6" }

	.text
foo:
	fcmp r1, f1, f2 ; { dg-error "can only use r0 as Dest register" }
	fcmp r0, f1, f2
	fcmp f1, f2
	.end
