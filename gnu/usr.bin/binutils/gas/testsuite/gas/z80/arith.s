	.text
	.org 0
;;; 8-bit arithmetic and logic
	add a,a
	add a,b
	add a,c
	add a,d
	add a,e
	add a,h
	add a,l
	add a,(hl)
	add a,(ix+5)
	add a,(iy+5)
	add a,17

	adc a,a
	adc a,b
	adc a,c
	adc a,d
	adc a,e
	adc a,h
	adc a,l
	adc a,(hl)
	adc a,(ix+5)
	adc a,(iy+5)
	adc a,17

	sub a
	sub b
	sub c
	sub d
	sub e
	sub h
	sub l
	sub (hl)
	sub (ix+5)
	sub (iy+5)
	sub 17

	sbc a,a
	sbc a,b
	sbc a,c
	sbc a,d
	sbc a,e
	sbc a,h
	sbc a,l
	sbc a,(hl)
	sbc a,(ix+5)
	sbc a,(iy+5)
	sbc a,17

	and a
	and b
	and c
	and d
	and e
	and h
	and l
	and (hl)
	and (ix+5)
	and (iy+5)
	and 17

	xor a
	xor b
	xor c
	xor d
	xor e
	xor h
	xor l
	xor (hl)
	xor (ix+5)
	xor (iy+5)
	xor 17

	or a
	or b
	or c
	or d
	or e
	or h
	or l
	or (hl)
	or (ix+5)
	or (iy+5)
	or 17

	cp a
	cp b
	cp c
	cp d
	cp e
	cp h
	cp l
	cp (hl)
	cp (ix+5)
	cp (iy+5)
	cp 17

	inc a
	inc b
	inc c
	inc d
	inc e
	inc h
	inc l
	inc (hl)
	inc (ix+5)
	inc (iy+5)

	dec a
	dec b
	dec c
	dec d
	dec e
	dec h
	dec l
	dec (hl)
	dec (ix+5)
	dec (iy+5)

;;; 16-bit arithmetic anmd logic
	add hl,bc
	add hl,de
	add hl,hl
	add hl,sp
	
	add ix,bc
	add ix,de
	add ix,ix
	add ix,sp
	
	add iy,bc
	add iy,de
	add iy,iy
	add iy,sp
	
	adc hl,bc
	adc hl,de
	adc hl,hl
	adc hl,sp

	sbc hl,bc
	sbc hl,de
	sbc hl,hl
	sbc hl,sp

	inc bc
	inc de
	inc hl
	inc sp
	inc ix
	inc iy

	dec bc
	dec de
	dec hl
	dec sp
	dec ix
	dec iy

	