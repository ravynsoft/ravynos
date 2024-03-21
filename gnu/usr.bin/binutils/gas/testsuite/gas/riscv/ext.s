target:
	.option arch, -c
	zext.b	a0, a0
	zext.h	a0, a0
	sext.b	a0, a0
	sext.h	a0, a0

	zext.b	a1, a2
	zext.h	a1, a2
	sext.b	a1, a2
	sext.h	a1, a2

.ifdef __64_bit__
	zext.w	a0, a0
	sext.w	a0, a0

	zext.w	a1, a2
	sext.w	a1, a2
.endif

	.option arch, +c
	zext.b	a0, a0
	zext.h	a0, a0
	sext.b	a0, a0
	sext.h	a0, a0

	zext.b	a1, a2
	zext.h	a1, a2
	sext.b	a1, a2
	sext.h	a1, a2

.ifdef __64_bit__
	zext.w	a0, a0
	sext.w	a0, a0

	zext.w	a1, a2
	sext.w	a1, a2
.endif
