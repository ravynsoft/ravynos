target:
	ld a0, (a0)  # 'Cl'
	ld a0, 0(a0) # 'Cl'
	sd a0, (a0)  # 'Cl'
	sd a0, 0(a0) # 'Cl'
	ld a0, (sp)  # 'Cn'
	ld a0, 0(sp) # 'Cn'
	sd a0, (sp)  # 'CN'
	sd a0, 0(sp) # 'CN'
