; Test error messages where targets are out of range.

; { dg-do assemble }
; { dg-options "-mtune=mcm" }

	.text
foo:
	fstore	r4,f15
	read.b	r6,1(r4)  ; { dg-error "base register not ready" "r4 not ready" }
	readmdc r5
	read.w	r7,31(r5) ; { dg-error "base register not ready" "r5 not ready" }
	fcmp	r6,f4,f5
	read.l	r8,13(r6) ; { dg-error "base register not ready" "r6 not ready" }
	move.b	r7,r0
	write.b	2(r7),r0  ; { dg-error "base register not ready" "r7 not ready" }
	move.w	r8,r0
	write.w	2(r8),r0  ; { dg-error "base register not ready" "r8 not ready" }
	move.l	r9,r0
	write.b	2(r9),r0  ; { dg-error "base register not ready" "r9 not ready" }
	.end
