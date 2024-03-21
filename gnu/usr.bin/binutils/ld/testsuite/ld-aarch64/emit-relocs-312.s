.comm   gempy,4
.comm	jempy,4
.comm	lempy,4
.text

	and	x0,x0,x0
	and	x0,x0,#0x1
        ldr     x4,tempy
	ldr	x7,tempy2
	ldr	x17,tempy3
	ldr	x3, [x2, #:got_lo12:jempy]
	ldr	x4, [x2, #:got_lo12:gempy]
	ldr	x5, [x2, #:got_lo12:lempy]
