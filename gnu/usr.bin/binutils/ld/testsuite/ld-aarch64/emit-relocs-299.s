.comm   gempy,4,4
.text

	and	x0,x0,x0
	and	x0,x0,#0x1
	ldr     q4, [x3, #:lo12:tempy]
	ldr	q7, [x3, #:lo12:tempy2]
	ldr	q17, [x3, #:lo12:tempy3]
