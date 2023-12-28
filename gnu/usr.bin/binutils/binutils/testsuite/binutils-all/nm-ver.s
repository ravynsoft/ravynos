	.symver foo_old,foo@VER_1
	.hidden foo_old
foo_old:
	.dc.b 0

	.symver foo_new,foo@@VER_2
	.global foo_new
foo_new:
	.dc.b 0
