	.text
foo:	.single	0r1.2345e+06
	.single 0f3.14159
	.double 0r2.718282
        .double .0000000000000000000001
        .double 1e-22

	.dc.s 1
	.dc.s 0f:1234
	.dc.s Inf
 .ifdef hasnan
	.dc.s NaN
	.dc.s QNaN
	.dc.s SNaN
 .endif
	.dcb.s 1
	.dcb.s 1, 1
	.dcb.s 1, 0s:4321
	.ds.s 1, -1

	.dc.d 1
	.dc.d 0d:1234
	.dc.d +Inf
 .ifdef hasnan
	.dc.d -NaN
	.dc.d +QNaN
	.dc.d -SNaN
 .endif
	.dcb.d 1
	.dcb.d 1, 1
	.dcb.d 1, 0r:4321
	.ds.d 1, -1

	.dc.x 0x:1234
	.dcb.x 1
	.dcb.x 1, 0x:4321
	.ds.x 1, -1
