	.text
	.global _start
_start:
	j	1f
	.rep	33000
	movi	a2, 0xf03df03d
	.endr
1:
	ret
