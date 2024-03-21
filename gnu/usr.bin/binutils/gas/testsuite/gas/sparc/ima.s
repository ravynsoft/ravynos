# Test IMA instructions
	.text
	fpmaddx  %f10, %f12, %f2, %f8
	fpmaddxhi  %f14, %f8, %f38, %f18
