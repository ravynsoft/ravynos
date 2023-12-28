        nop
foo:
	nop
	nop
        brclr  d0, #1, foo
        brset  d1, d2, foo
        brclr.b (x+), #2, foo
        brset.b (23,d0), d0, foo
	brclr.b  902, #2, foo
