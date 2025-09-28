	.text
	.global __start
__start:
	lrs.b	r10, [word1]
	lrs.h	r10, [word1]
	lrs.w	r10, [word1]

	srs.b	r11, [word1]
	srs.h	r11, [word1]
	srs.w	r11, [word1]
