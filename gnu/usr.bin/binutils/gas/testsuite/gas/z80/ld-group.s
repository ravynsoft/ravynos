	.section .text
	.org 0
	;; 8-bit load group
	ld a,i
	ld a,r
	ld a,a
	ld a,b
	ld a,c
	ld a,d
	ld a,e
	ld a,h
	ld a,l
	ld a,(hl)
	ld a,(bc)
	ld a,(de)
	ld a,(ix+5)
	ld a,(iy+5)
	ld a,(0x8405)
	ld a,0x11

	ld b,a
	ld b,b
	ld b,c
	ld b,d
	ld b,e
	ld b,h
	ld b,l
	ld b,(hl)
	ld b,(ix+5)
	ld b,(iy+5)
	ld b,0x11

	ld c,a
	ld c,b
	ld c,c
	ld c,d
	ld c,e
	ld c,h
	ld c,l
	ld c,(hl)
	ld c,(ix+5)
	ld c,(iy+5)
	ld c,0x11

	ld d,a
	ld d,b
	ld d,c
	ld d,d
	ld d,e
	ld d,h
	ld d,l
	ld d,(hl)
	ld d,(ix+5)
	ld d,(iy+5)
	ld d,0x11
	
	ld e,a
	ld e,b
	ld e,c
	ld e,d
	ld e,e
	ld e,h
	ld e,l
	ld e,(hl)
	ld e,(ix+5)
	ld e,(iy+5)
	ld e,0x11

	ld h,a
	ld h,b
	ld h,c
	ld h,d
	ld h,e
	ld h,h
	ld h,l
	ld h,(hl)
	ld h,(ix+5)
	ld h,(iy+5)
	ld h,0x11
	
	ld l,a
	ld l,b
	ld l,c
	ld l,d
	ld l,e
	ld l,h
	ld l,l
	ld l,(hl)
	ld l,(ix+5)
	ld l,(iy+5)
	ld l,0x11

	ld (hl),a
	ld (hl),b
	ld (hl),c
	ld (hl),d
	ld (hl),e
	ld (hl),h
	ld (hl),l
	ld (hl),0x11
	
	ld (bc),a
	ld (de),a

	ld (ix+5),a
	ld (ix+5),b
	ld (ix+5),c
	ld (ix+5),d
	ld (ix+5),e
	ld (ix+5),h
	ld (ix+5),l
	ld (ix+5),0x11

	ld (iy+5),a
	ld (iy+5),b
	ld (iy+5),c
	ld (iy+5),d
	ld (iy+5),e
	ld (iy+5),h
	ld (iy+5),l
	ld (iy+5),0x11

	ld (0x8407),a
	ld i,a
	ld r,a

	;;  16-bit load group --- ld, pop and push
	pop af
	
	ld bc,0x8405
	ld bc,(0x8405)
	pop bc
	
	ld de,0x8405
	ld de,(0x8405)
	pop de
	
	ld hl,0x8405
	ld hl,(0x8405)
	pop hl
	
	ld sp,hl
	ld sp,ix
	ld sp,iy
	ld sp,0x8402
	ld sp,(0x8302)
	
	ld ix,0x8405
	ld ix,(0x8405)
	pop ix
	
	ld iy,0x8405
	ld iy,(0x8405)
	pop iy
	
	ld (0x8432),bc
	ld (0x8432),de
	ld (0x8432),hl
	ld (0x8432),sp
	ld (0x8432),ix
	ld (0x8432),iy

	push af
	push bc
	push de
	push hl
	push ix
	push iy

