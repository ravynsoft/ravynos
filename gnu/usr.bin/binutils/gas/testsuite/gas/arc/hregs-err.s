; { dg-do assemble { target arc*-*-* } }
	.cpu	HS
	.text
	ld_s	r0,[r32,28]	; { dg-error "Error: register must be GP for instruction 'ld_s'" }
	ld_s	r0,[r28,28]
	ld_s	r1,[r32,28]	; { dg-error "Error: register must be GP for instruction 'ld_s'" }
	ld_s	r2,[r32,28]	; { dg-error "Error: register must be PCL for instruction 'ld_s'" }
	ld_s	r3,[pcl,0x10]
	add_s	r0,r0,r32	; { dg-error "Error: register out of range for instruction 'add_s'" }
	add_s	r0,r0,r28
	mov_s.ne r0,r32		; { dg-error "Error: register out of range for instruction 'mov_s'" }
