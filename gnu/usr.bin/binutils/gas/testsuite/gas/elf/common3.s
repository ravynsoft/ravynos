	.comm foobar,30,4
	.comm buf1, 5-1, 8
	.comm buf2, 4, 9-1
	.ifndef lcomm_align
	.lcomm lbuf, 9-1
	.else
	.lcomm lbuf, 9-1, 8
	.endif
