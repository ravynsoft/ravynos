target:
	lw a0, (a0)  # 'Ck'
	lw a0, 0(a0) # 'Ck'
	sw a0, (a0)  # 'Ck'
	sw a0, 0(a0) # 'Ck'
	lw a0, (sp)  # 'Cm'
	lw a0, 0(sp) # 'Cm'
	sw a0, (sp)  # 'CM'
	sw a0, 0(sp) # 'CM'
