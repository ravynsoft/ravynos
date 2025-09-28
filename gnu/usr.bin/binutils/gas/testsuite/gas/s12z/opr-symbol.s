#sym1: set $123456

#sym2: set $abcdef

	.equiv sym1, 0x123456
	.equiv sym2, 0xabcdef

	mov.b #23, sym1
	mov.w #23, sym1
	mov.p #23, sym1
	mov.l #23, sym1

	mov.b sym1, sym2
	mov.w sym1, sym2
	mov.p sym1, sym2
	mov.l sym1, sym2

	add d1, sym1
	and d2, sym1
	sub d3, sym1
	or  d4, sym1

	inc.b  sym1
	inc.w  sym1
	inc.l  sym1

	ld d4, sym1

	jmp sym2
	jsr sym1

	dec.b  sym1
	dec.w  sym1
	dec.l  sym1

	clr.b  sym1
	clr.w  sym1
	clr.l  sym1

	st d5, sym1

	com.b  sym1
	com.w  sym1
	com.l  sym1

	neg.b  sym1
	neg.w  sym1
	neg.l  sym1

	cmp d5, sym1

	ld s, sym1
	st s, sym1
	cmp s, sym1

	minu d2, sym1
	maxu d2, sym1

	mins d2, sym1
	maxs d2, sym1

	adc d3, sym1
	bit d3, sym1
	sbc d3, sym1
	eor d3, sym1

	brclr.b sym1, d1, *+3
	brclr.w sym1, #2, *+4

	brset.b sym2, d2, *+5
	brset.w sym2, #3, *+6

	mulu.b d1, d2, sym1
	mulu.ll d6, sym1, sym2

	muls.b d1, d2, sym1
	muls.ll d6, sym1, sym2

	qmuls.b d1, d2, sym1
	qmuls.ll d6, sym1, sym2

	divu.b d1, d2, sym1
	divu.ll d6, sym1, sym2

	divs.b d1, d2, sym1
	divs.ll d6, sym1, sym2

	bclr.b sym1, #2
	bclr.w sym2, d2

	bset.b sym1, #2
	bset.w sym2, d2

	btgl.b sym1, #2
	btgl.w sym2, d2

	tbne.b sym1, *+8
	dbpl.w sym2, *+9

	bfins.b d2, sym1, d2
	bfins.w sym1, d2, d2
	bfins.p d2, sym1, #8:1
	bfins.l sym1, d2, #8:1

	bfext.b d2, sym1, d2
	bfext.w sym1, d2, d2
	bfext.p d2, sym1, #8:1
	bfext.l sym1, d2, #7:2

