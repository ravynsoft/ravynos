; Test ARC specific assembler warnings
;
; { dg-do assemble { target arc*-*-* } }

	b.d foo
	mov r0,256

	j.d foo		; { dg-error "Error: flag mismatch for instruction 'j'" }
	mov r0,r1

foo:

