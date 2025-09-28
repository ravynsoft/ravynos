	.cpu HS
	.text
	add_s	r0,r0,0x1000
	add_s	0,0x1001,1
	cmp_s	r2,0x1000
	cmp_s	0x1000,1
	mov_s	r2,0x1000
	mov_s	0,0x1000
	mov_s.ne	r2,0x1000
