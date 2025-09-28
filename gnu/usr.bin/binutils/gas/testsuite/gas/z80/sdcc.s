        .module longpolls
        .optsdcc -mz80

valueadr = 0x1234

	.globl	function
	.globl	_start
	.globl	_finish

	.area	_DABS (ABS)
	.area	_HOME
	.area	_CODE
_start::
;comment
	ld      hl, #4+0
00000$:
	adc	a, a
	adc	a, b
	adc	a, c
	adc	a, d
	adc	a, e
	adc	a, h
	adc	a, l
	adc	a, ixh
	adc	a, ixl
	adc	a, iyh
	adc	a, iyl
	adc	a, #0xa5
	adc	a, (hl)
	adc	a, 5 (ix)
	adc	a, -2 (iy)
00100$:
	add	a, a
	add	a, b
	add	a, c
	add	a, d
	add	a, e
	add	a, h
	add	a, l
	add	a, ixh
	add	a, ixl
	add	a, iyh
	add	a, iyl
	add	a, #0xa5
	add	a, (hl)
	add	a, 5 (ix)
	add	a, -2 (iy)
00200$:
	and	a, a
	and	a, b
	and	a, c
	and	a, d
	and	a, e
	and	a, h
	and	a, l
	and	a, ixh
	and	a, ixl
	and	a, iyh
	and	a, iyl
	and	a, #0xa5
	and	a, (hl)
	and	a, 5 (ix)
	and	a, -2 (iy)
00300$:
	cp	a, a
	cp	a, b
	cp	a, c
	cp	a, d
	cp	a, e
	cp	a, h
	cp	a, l
	cp	a, ixh
	cp	a, ixl
	cp	a, iyh
	cp	a, iyl
	cp	a, #0xa5
	cp	a, (hl)
	cp	a, 5 (ix)
	cp	a, -2 (iy)
00400$:
	or	a, a
	or	a, b
	or	a, c
	or	a, d
	or	a, e
	or	a, h
	or	a, l
	or	a, ixh
	or	a, ixl
	or	a, iyh
	or	a, iyl
	or	a, #0xa5
	or	a, (hl)
	or	a, 5 (ix)
	or	a, -2 (iy)
00500$:
	sbc	a, a
	sbc	a, b
	sbc	a, c
	sbc	a, d
	sbc	a, e
	sbc	a, h
	sbc	a, l
	sbc	a, ixh
	sbc	a, ixl
	sbc	a, iyh
	sbc	a, iyl
	sbc	a, #0xa5
	sbc	a, (hl)
	sbc	a, 5 (ix)
	sbc	a, -2 (iy)
00600$:
	sub	a, a
	sub	a, b
	sub	a, c
	sub	a, d
	sub	a, e
	sub	a, h
	sub	a, l
	sub	a, ixh
	sub	a, ixl
	sub	a, iyh
	sub	a, iyl
	sub	a, #0xa5
	sub	a, (hl)
	sub	a, 5 (ix)
	sub	a, -2 (iy)
00700$:
	xor	a, a
	xor	a, b
	xor	a, c
	xor	a, d
	xor	a, e
	xor	a, h
	xor	a, l
	xor	a, ixh
	xor	a, ixl
	xor	a, iyh
	xor	a, iyl
	xor	a, #0xa5
	xor	a, (hl)
	xor	a, 5 (ix)
	xor	a, -2 (iy)

	jp	0$
	jp	100$
	jp	200$
	jp	300$
	jp	500$
	jp	600$
	jp	700$
_func:
	ld	hl,0
	ld	(hl),#<function
00100$:
	inc	hl
	ld	(hl),#>function
00600$:
	jr	100$
_finish::
	ld	a, 2 (iy)
	ld	-1 (ix), a
	ld	a, (#valueadr+#0)
	ret
	.dw	#0x1f27
	.db	#0x2f
	.end
