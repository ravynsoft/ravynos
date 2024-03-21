	.text
	.org 0
	;; eZ80 instructions

; AND A,x group
	and	a,a
	and	a,b
	and	a,c
	and	a,d
	and	a,e
	and	a,h
	and	a,l
	and	a,(hl)
	and	a,0xaa
	and	a,(ix+5)
	and	a,(iy-5)
; CP A,x group
	cp	a,a
	cp	a,b
	cp	a,c
	cp	a,d
	cp	a,e
	cp	a,h
	cp	a,l
	cp	a,(hl)
	cp	a,0xaa
	cp	a,(ix+5)
	cp	a,(iy-5)

; OR A,x group
	or	a,a
	or	a,b
	or	a,c
	or	a,d
	or	a,e
	or	a,h
	or	a,l
	or	a,(hl)
	or	a,0xaa
	or	a,(ix+5)
	or	a,(iy-5)

; SUB A,x group
	sub	a,a
	sub	a,b
	sub	a,c
	sub	a,d
	sub	a,e
	sub	a,h
	sub	a,l
	sub	a,(hl)
	sub	a,0xaa
	sub	a,(ix+5)
	sub	a,(iy-5)

; TST A,x group
	tst	a,a
	tst	a,b
	tst	a,c
	tst	a,d
	tst	a,e
	tst	a,h
	tst	a,l
	tst	a,(hl)
	tst	a,0x0f

; XOR A,x group
	xor	a,a
	xor	a,b
	xor	a,c
	xor	a,d
	xor	a,e
	xor	a,h
	xor	a,l
	xor	a,(hl)
	xor	a,0xaa
	xor	a,(ix+5)
	xor	a,(iy-5)

; IN r,(BC) group (new naming)
	in a,(bc)
	in b,(bc)
	in c,(bc)
	in d,(bc)
	in e,(bc)
	in h,(bc)
	in l,(bc)

; OUT (BC),r group (new naming)
	out (bc),a
	out (bc),b
	out (bc),c
	out (bc),d
	out (bc),e
	out (bc),h
	out (bc),l

; LD rr,(ii+d) group
	ld	bc,(ix-7)
	ld	de,(ix-7)
	ld	hl,(ix-7)
	ld	ix,(ix-7)
	ld	iy,(ix-7)

	ld	bc,(iy+38)
	ld	de,(iy+38)
	ld	hl,(iy+38)
	ld	ix,(iy+38)
	ld	iy,(iy+38)

; LD (ii+d),rr group
	ld	(ix+126),bc
	ld	(ix+126),de
	ld	(ix+126),hl
	ld	(ix+126),ix
	ld	(ix+126),iy
	ld	(iy-98),bc
	ld	(iy-98),de
	ld	(iy-98),hl
	ld	(iy-98),ix
	ld	(iy-98),iy

; LEA rr,ii+d group
	lea	bc,ix-27
	lea	de,ix-27
	lea	hl,ix-27
	lea	ix,ix-27
	lea	iy,ix-27
	lea	bc,iy+12
	lea	de,iy+12
	lea	hl,iy+12
	lea	ix,iy+12
	lea	iy,iy+12

; PEA ii+d group
	pea	ix+127
	pea	iy-128

; IN0 group
	in0 a,(0x5)
	in0 b,(0x5)
	in0 c,(0x5)
	in0 d,(0x5)
	in0 e,(0x5)
	in0 h,(0x5)
	in0 l,(0x5)

; OUT0 group
	out0 (0x5),a
	out0 (0x5),b
	out0 (0x5),c
	out0 (0x5),d
	out0 (0x5),e
	out0 (0x5),h
	out0 (0x5),l

; MLT group
	mlt bc
	mlt de
	mlt hl
	mlt sp

; TSTIO instruction
	tstio 0f0h

; SLP instruction
	slp

; ADLMIX flag manipulation instructions
	stmix
	rsmix

; Additional block I/O instructions
	inim
	otim
	ini2
	indm
	otdm
	ind2
	inimr
	otimr
	ini2r
	indmr
	otdmr
	ind2r
	outi2
	outd2
	oti2r
	otd2r
	inirx
	otirx
	indrx
	otdrx

; Index registers halves
	ld a,ixh
	ld b,ixh
	ld c,ixh
	ld d,ixh
	ld e,ixh
	ld ixh,ixh
	ld ixl,ixh
	ld a,ixl
	ld b,ixl
	ld c,ixl
	ld d,ixl
	ld e,ixl
	ld ixh,ixl
	ld ixl,ixl
	ld a,iyh
	ld b,iyh
	ld c,iyh
	ld d,iyh
	ld e,iyh
	ld iyh,iyh
	ld iyl,iyh
	ld a,iyl
	ld b,iyl
	ld c,iyl
	ld d,iyl
	ld e,iyl
	ld iyh,iyl
	ld iyl,iyl
	ld ixh,a
	ld ixh,b
	ld ixh,c
	ld ixh,d
	ld ixh,e
	ld ixh,ixh
	ld ixh,ixl
	ld ixh,25
	ld ixl,a
	ld ixl,b
	ld ixl,c
	ld ixl,d
	ld ixl,e
	ld ixl,ixh
	ld ixl,ixl
	ld ixl,25
	ld iyh,a
	ld iyh,b
	ld iyh,c
	ld iyh,d
	ld iyh,e
	ld iyh,iyh
	ld iyh,iyl
	ld iyh,25
	ld iyl,a
	ld iyl,b
	ld iyl,c
	ld iyl,d
	ld iyl,e
	ld iyl,iyh
	ld iyl,iyl
	ld iyl,25
	add a,ixh
	add a,ixl
	add a,iyh
	add a,iyl
	adc a,ixh
	adc a,ixl
	adc a,iyh
	adc a,iyl
	cp a,ixh
	cp a,ixl
	cp a,iyh
	cp a,iyl
	dec ixh
	dec ixl
	dec iyh
	dec iyl
	inc ixh
	inc ixl
	inc iyh
	inc iyl
	sbc a,ixh
	sbc a,ixl
	sbc a,iyh
	sbc a,iyl
	sub a,ixh
	sub a,ixl
	sub a,iyh
	sub a,iyl
	and a,ixh
	and a,ixl
	and a,iyh
	and a,iyl
	or a,ixh
	or a,ixl
	or a,iyh
	or a,iyl
	xor a,ixh
	xor a,ixl
	xor a,iyh
	xor a,iyl

; Special ADL mode only instructions
	ld i,hl
	ld hl,i
	ld mb,a
	ld a,mb

; Standard Z80 instructions

	adc a,(hl)
	adc a,(ix+9)
	adc a,(iy+9)
	adc a,3
	adc a,a
	adc a,b
	adc a,c
	adc a,d
	adc a,e
	adc a,h
	adc a,l
	adc hl,bc
	adc hl,de
	adc hl,hl
	adc hl,sp
	add a,(hl)
	add a,(ix+9)
	add a,(iy+9)
	add a,3
	add a,a
	add a,b
	add a,c
	add a,d
	add a,e
	add a,h
	add a,l
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
	and (hl)
	and (ix+9)
	and (iy+9)
	and 3
	and a
	and b
	and c
	and d
	and e
	and h
	and l
	bit 0,(hl)
	bit 0,(ix+9)
	bit 0,(iy+9)
	bit 0,a
	bit 0,b
	bit 0,c
	bit 0,d
	bit 0,e
	bit 0,h
	bit 0,l
	bit 1,(hl)
	bit 1,(ix+9)
	bit 1,(iy+9)
	bit 1,a
	bit 1,b
	bit 1,c
	bit 1,d
	bit 1,e
	bit 1,h
	bit 1,l
	bit 2,(hl)
	bit 2,(ix+9)
	bit 2,(iy+9)
	bit 2,a
	bit 2,b
	bit 2,c
	bit 2,d
	bit 2,e
	bit 2,h
	bit 2,l
	bit 3,(hl)
	bit 3,(ix+9)
	bit 3,(iy+9)
	bit 3,a
	bit 3,b
	bit 3,c
	bit 3,d
	bit 3,e
	bit 3,h
	bit 3,l
	bit 4,(hl)
	bit 4,(ix+9)
	bit 4,(iy+9)
	bit 4,a
	bit 4,b
	bit 4,c
	bit 4,d
	bit 4,e
	bit 4,h
	bit 4,l
	bit 5,(hl)
	bit 5,(ix+9)
	bit 5,(iy+9)
	bit 5,a
	bit 5,b
	bit 5,c
	bit 5,d
	bit 5,e
	bit 5,h
	bit 5,l
	bit 6,(hl)
	bit 6,(ix+9)
	bit 6,(iy+9)
	bit 6,a
	bit 6,b
	bit 6,c
	bit 6,d
	bit 6,e
	bit 6,h
	bit 6,l
	bit 7,(hl)
	bit 7,(ix+9)
	bit 7,(iy+9)
	bit 7,a
	bit 7,b
	bit 7,c
	bit 7,d
	bit 7,e
	bit 7,h
	bit 7,l
	call 0x123456
	call c,0x123456
	call m,0x123456
	call nc,0x123456
	call nz,0x123456
	call p,0x123456
	call pe,0x123456
	call po,0x123456
	call z,0x123456
	ccf
	cp (hl)
	cp (ix+9)
	cp (iy+9)
	cp 03
	cp a
	cp b
	cp c
	cp d
	cp e
	cp h
	cp l
	cpd
	cpdr
	cpi
	cpir
	cpl
	daa
	dec (hl)
	dec (ix+9)
	dec (iy+9)
	dec a
	dec b
	dec bc
	dec c
	dec d
	dec de
	dec e
	dec h
	dec hl
	dec ix
	dec iy
	dec l
	dec sp
	di
	djnz .+7
	ei
	ex (sp),hl
	ex (sp),ix
	ex (sp),iy
	ex af,af'	;'
	ex de,hl
	exx
	halt
	im 0
	im 1
	im 2
	in a,(c)
	in a,(3)
	in b,(c)
	in c,(c)
	in d,(c)
	in e,(c)
	in h,(c)
	in l,(c)
	inc (hl)
	inc (ix+9)
	inc (iy+9)
	inc a
	inc b
	inc bc
	inc c
	inc d
	inc de
	inc e
	inc h
	inc hl
	inc ix
	inc iy
	inc l
	inc sp
	ind
	indr
	ini
	inir
	jp (hl)
	jp (ix)
	jp (iy)
	jp 0x123456
	jp c,0x123456
	jp m,0x123456
	jp nc,0x123456
	jp nz,0x123456
	jp p,0x123456
	jp pe,0x123456
	jp po,0x123456
	jp z,0x123456
	jr .+7
	jr c,.+7
	jr nc,.+7
	jr nz,.+7
	jr z,.+7
	ld (0x123456),a
	ld (0x123456),bc
	ld (0x123456),de
	ld (0x123456),hl
	ld (0x123456),ix
	ld (0x123456),iy
	ld (0x123456),sp
	ld (bc),a
	ld (de),a
	ld (hl),3
	ld (hl),a
	ld (hl),b
	ld (hl),c
	ld (hl),d
	ld (hl),e
	ld (hl),h
	ld (hl),l
	ld (ix+9),3
	ld (ix+9),a
	ld (ix+9),b
	ld (ix+9),c
	ld (ix+9),d
	ld (ix+9),e
	ld (ix+9),h
	ld (ix+9),l
	ld (iy+9),3
	ld (iy+9),a
	ld (iy+9),b
	ld (iy+9),c
	ld (iy+9),d
	ld (iy+9),e
	ld (iy+9),h
	ld (iy+9),l
	ld a,(0x123456)
	ld a,(bc)
	ld a,(de)
	ld a,(hl)
	ld a,(ix+9)
	ld a,(iy+9)
	ld a,3
	ld a,a
	ld a,b
	ld a,c
	ld a,d
	ld a,e
	ld a,h
	ld a,i
	ld a,l
	ld a,r
	ld b,(hl)
	ld b,(ix+9)
	ld b,(iy+9)
	ld b,3
	ld b,a
	nop ;ld b,b
	ld b,c
	ld b,d
	ld b,e
	ld b,h
	ld b,l
	ld bc,(0x123456)
	ld bc,0x123456
	ld c,(hl)
	ld c,(ix+9)
	ld c,(iy+9)
	ld c,3
	ld c,a
	ld c,b
	nop ;ld c,c
	ld c,d
	ld c,e
	ld c,h
	ld c,l
	ld d,(hl)
	ld d,(ix+9)
	ld d,(iy+9)
	ld d,3
	ld d,a
	ld d,b
	ld d,c
	nop ;ld d,d
	ld d,e
	ld d,h
	ld d,l
	ld de,(0x123456)
	ld de,0x123456
	ld e,(hl)
	ld e,(ix+9)
	ld e,(iy+9)
	ld e,3
	ld e,a
	ld e,b
	ld e,c
	ld e,d
	nop ;ld e,e
	ld e,h
	ld e,l
	ld h,(hl)
	ld h,(ix+9)
	ld h,(iy+9)
	ld h,3
	ld h,a
	ld h,b
	ld h,c
	ld h,d
	ld h,e
	ld h,h
	ld h,l
	ld hl,(0x123456)
	ld hl,0x123456
	ld i,a
	ld ix,(0x123456)
	ld ix,0x123456
	ld iy,(0x123456)
	ld iy,0x123456
	ld l,(hl)
	ld l,(ix+9)
	ld l,(iy+9)
	ld l,3
	ld l,a
	ld l,b
	ld l,c
	ld l,d
	ld l,e
	ld l,h
	ld l,l
	ld r,a
	ld sp,(0x123456)
	ld sp,0x123456
	ld sp,hl
	ld sp,ix
	ld sp,iy
	ldd
	lddr
	ldi
	ldir
	neg
	nop
	or (hl)
	or (ix+9)
	or (iy+9)
	or 3
	or a
	or b
	or c
	or d
	or e
	or h
	or l
	otdr
	otir
	out (c),a
	out (c),b
	out (c),c
	out (c),d
	out (c),e
	out (c),h
	out (c),l
	out (3),a
	outd
	outi
	pop af
	pop bc
	pop de
	pop hl
	pop ix
	pop iy
	push af
	push bc
	push de
	push hl
	push ix
	push iy
	res 0,(hl)
	res 0,(ix+9)
	res 0,(iy+9)
	res 0,a
	res 0,b
	res 0,c
	res 0,d
	res 0,e
	res 0,h
	res 0,l
	res 1,(hl)
	res 1,(ix+9)
	res 1,(iy+9)
	res 1,a
	res 1,b
	res 1,c
	res 1,d
	res 1,e
	res 1,h
	res 1,l
	res 2,(hl)
	res 2,(ix+9)
	res 2,(iy+9)
	res 2,a
	res 2,b
	res 2,c
	res 2,d
	res 2,e
	res 2,h
	res 2,l
	res 3,(hl)
	res 3,(ix+9)
	res 3,(iy+9)
	res 3,a
	res 3,b
	res 3,c
	res 3,d
	res 3,e
	res 3,h
	res 3,l
	res 4,(hl)
	res 4,(ix+9)
	res 4,(iy+9)
	res 4,a
	res 4,b
	res 4,c
	res 4,d
	res 4,e
	res 4,h
	res 4,l
	res 5,(hl)
	res 5,(ix+9)
	res 5,(iy+9)
	res 5,a
	res 5,b
	res 5,c
	res 5,d
	res 5,e
	res 5,h
	res 5,l
	res 6,(hl)
	res 6,(ix+9)
	res 6,(iy+9)
	res 6,a
	res 6,b
	res 6,c
	res 6,d
	res 6,e
	res 6,h
	res 6,l
	res 7,(hl)
	res 7,(ix+9)
	res 7,(iy+9)
	res 7,a
	res 7,b
	res 7,c
	res 7,d
	res 7,e
	res 7,h
	res 7,l
	ret
	ret c
	ret m
	ret nc
	ret nz
	ret p
	ret pe
	ret po
	ret z
	reti
	retn
	rl (hl)
	rl (ix+9)
	rl (iy+9)
	rl a
	rl b
	rl c
	rl d
	rl e
	rl h
	rl l
	rla
	rlc (hl)
	rlc (ix+9)
	rlc (iy+9)
	rlc a
	rlc b
	rlc c
	rlc d
	rlc e
	rlc h
	rlc l
	rlca
	rld
	rr (hl)
	rr (ix+9)
	rr (iy+9)
	rr a
	rr b
	rr c
	rr d
	rr e
	rr h
	rr l
	rra
	rrc (hl)
	rrc (ix+9)
	rrc (iy+9)
	rrc a
	rrc b
	rrc c
	rrc d
	rrc e
	rrc h
	rrc l
	rrca
	rrd
	rst 0x00
	rst 0x08
	rst 0x10
	rst 0x18
	rst 0x20
	rst 0x28
	rst 0x30
	rst 0x38
	sbc a,(hl)
	sbc a,(ix+9)
	sbc a,(iy+9)
	sbc a,3
	sbc a,a
	sbc a,b
	sbc a,c
	sbc a,d
	sbc a,e
	sbc a,h
	sbc a,l
	sbc hl,bc
	sbc hl,de
	sbc hl,hl
	sbc hl,sp
	scf
	set 0,(hl)
	set 0,(ix+9)
	set 0,(iy+9)
	set 0,a
	set 0,b
	set 0,c
	set 0,d
	set 0,e
	set 0,h
	set 0,l
	set 1,(hl)
	set 1,(ix+9)
	set 1,(iy+9)
	set 1,a
	set 1,b
	set 1,c
	set 1,d
	set 1,e
	set 1,h
	set 1,l
	set 2,(hl)
	set 2,(ix+9)
	set 2,(iy+9)
	set 2,a
	set 2,b
	set 2,c
	set 2,d
	set 2,e
	set 2,h
	set 2,l
	set 3,(hl)
	set 3,(ix+9)
	set 3,(iy+9)
	set 3,a
	set 3,b
	set 3,c
	set 3,d
	set 3,e
	set 3,h
	set 3,l
	set 4,(hl)
	set 4,(ix+9)
	set 4,(iy+9)
	set 4,a
	set 4,b
	set 4,c
	set 4,d
	set 4,e
	set 4,h
	set 4,l
	set 5,(hl)
	set 5,(ix+9)
	set 5,(iy+9)
	set 5,a
	set 5,b
	set 5,c
	set 5,d
	set 5,e
	set 5,h
	set 5,l
	set 6,(hl)
	set 6,(ix+9)
	set 6,(iy+9)
	set 6,a
	set 6,b
	set 6,c
	set 6,d
	set 6,e
	set 6,h
	set 6,l
	set 7,(hl)
	set 7,(ix+9)
	set 7,(iy+9)
	set 7,a
	set 7,b
	set 7,c
	set 7,d
	set 7,e
	set 7,h
	set 7,l
	sla (hl)
	sla (ix+9)
	sla (iy+9)
	sla a
	sla b
	sla c
	sla d
	sla e
	sla h
	sla l
	sra (hl)
	sra (ix+9)
	sra (iy+9)
	sra a
	sra b
	sra c
	sra d
	sra e
	sra h
	sra l
	srl (hl)
	srl (ix+9)
	srl (iy+9)
	srl a
	srl b
	srl c
	srl d
	srl e
	srl h
	srl l
	sub (hl)
	sub (ix+9)
	sub (iy+9)
	sub 3
	sub a
	sub b
	sub c
	sub d
	sub e
	sub h
	sub l
	xor (hl)
	xor (ix+9)
	xor (iy+9)
	xor 3
	xor a
	xor b
	xor c
	xor d
	xor e
	xor h
	xor l
	ld	bc,(hl)
	ld	de,(hl)
	ld	hl,(hl)
	ld	ix,(hl)
	ld	iy,(hl)
	ld	(hl),bc
	ld	(hl),de
	ld	(hl),hl
	ld	(hl),ix
	ld	(hl),iy
