// mov.s Test file for AArch64 mov aliases.
// This test file is also used for the mov-no-aliases test.

	.text

	// MOV/xr Xd|SP, Xn|SP
	//    Move (extended register) is an alias for ADD/xi Xd,Xn,#0, but
	//    only when one or other of the registers is SP. In other cases
	//    the ORR/xr Xd,#0,Xn instruction will be used.

	mov	w15, wsp
	mov	x15, sp
	mov	wsp, w7
	mov	sp, x7
	mov	wsp, wsp
	mov	sp, sp

	mov	x7, x15
	mov	w7, w15

	mov	w1, 88
	mov	w0, -1

	mov	x0, -4294967296

	mov	sp, #15
	mov	wsp, #15
	mov	xzr, #15
	mov	wzr, #15

	mov	w7, v15.s[3]
	mov	x15, v31.d[1]

	mov	x0, $$5
.set $$5, 0xff

	// ORR w0,w0,#0x99999999 with a non-standard encoding, i.e. the top
	// 4 bits in the 'immr' field is non-zero.  The top bits are ignored
	// during the decoding.
	.inst	0x320de400
