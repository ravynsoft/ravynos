	.text
	.org 0
;;; branch instructions
2:	
	jp 0x8405
	jp c,0x8405
	jp nc,0x8405
	jp z,0x8405
	jp nz,0x8405
	jp pe,0x8405
	jp po,0x8405
	jp m,0x8405
	jp p,0x8405

	jr 2b
	jr c,2b
	jr nc,2b
	jr z,2b
	jr nz,2b

	jp (hl)
	jp (ix)
	jp (iy)

	call 0x8405
	call c,0x8405
	call nc,0x8405
	call z,0x8405
	call nz,0x8405
	call pe,0x8405
	call po,0x8405
	call m,0x8405
	call p,0x8405
	
	djnz 2b

	ret
	ret c
	ret nc
	ret z
	ret nz
	ret pe
	ret po
	ret m
	ret p

	reti
	retn

	rst 0h
	rst 8h
	rst 10h
	rst 18h
	rst 20h
	rst 28h
	rst 30h
	rst 38h
