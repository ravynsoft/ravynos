	.text
	.arch generic32
equ:
	.set	xBX, %ebx

	.code16
	.arch i286
	inc	xBX
	incb	(xBX)
