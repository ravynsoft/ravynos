target:
	mul	a0, a1, a2
	mulh	a0, a1, a2
	mulhsu	a0, a1, a2
	mulhu	a0, a1, a2
.ifndef zmmul
	div	a0, a1, a2
	divu	a0, a1, a2
	rem	a0, a1, a2
	remu	a0, a1, a2
.endif

.ifdef rv64
	mulw	a0, a1, a2
.ifndef zmmul
	divw	a0, a1, a2
	divuw	a0, a1, a2
	remw	a0, a1, a2
	remuw	a0, a1, a2
.endif
.endif
