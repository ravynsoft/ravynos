.set a, 0x1
.set b, 0x1000
.set c, 0x1001
.set d, 0xfff
.set e, 0x7fffffff
.set g, 0x0
.set h, 0xffffffff
.text
	lla a0, a
	lla a0, b
	lla a0, c
	lla a0, d
	lla a0, e
	lla a0, g
	lla a0, h
