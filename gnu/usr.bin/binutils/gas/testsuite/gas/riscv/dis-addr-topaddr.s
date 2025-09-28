.ifdef rv64
topbase = 0xffffffff00000000
.else
topbase = 0
.endif

.set addr_top, topbase + 0xffffffff  # -1

target:
	lb	t0, -1(zero)
