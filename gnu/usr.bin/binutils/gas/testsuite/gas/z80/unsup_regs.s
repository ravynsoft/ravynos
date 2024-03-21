	.text
	.org	0
_start:
.ifdef NO_XYHL
	ld	a,ixl
ixl:
	ld	b,ixh
ixh:
	ld	c,iyl
iyl:
	ld	d,iyh
iyh:
.else
	ld	a,ixl
	ld	ixh,a
	ld	b,ixh
	ld	ixl,b
	ld	c,iyl
	ld	iyh,c
	ld	d,iyh
	ld	iyl,d
.endif

.ifdef NO_REG_F
f:	ld	a,f
.else
	in	f,(c)
.endif

.ifdef NO_INDEX
ix:	ld	hl,ix
iy:	ld	bc,iy
	ld	a,(ix-1)
	ld	(iy+1),a
.else
	add	ix,ix
	add	a,(ix+1)
	ld	ix,0x1234
	ld	iy,0x4321
	ld	(0x1234),iy
	ld	(iy-1),a
.endif

.ifdef NO_REG_R
r:	ld	a,r
	ld	(r),a
.else
	ld	a,r
	ld	r,a
.endif

.ifdef NO_REG_I
i:	ld	a,i
	ld	(i),a
.else
	ld	a,i
	ld	i,a
.endif

.ifndef EZ80
mb:	ld	hl,mb
	ld	a,mb
	ld	(mb),a
.else
	.assume	ADL=1
	ld	a,mb
	ld	mb,a
	.assume	ADL=0
.endif

	.END
