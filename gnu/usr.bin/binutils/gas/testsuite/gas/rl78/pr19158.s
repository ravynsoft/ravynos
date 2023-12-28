	.text
.set sfr, 0xffff8
	mov     a, sfr
	mov     sfr, a
	mov     sfr, #123
	xch     a, sfr
	
.set sfrp, 0xffff8
	movw    ax, sfrp
	movw    sfrp, ax
	movw    sfrp, #0x1234

	xch	a, 0xffffa
	xch	a, 0xffff9
	xch	a, 0xffffc
	xch	a, 0xffffd
	xch	a, 0xffffe
	xch	a, 0xfffff

	