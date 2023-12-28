	.section .foo,"a"
	.dc.a 0

	.section .moo,"ao",%progbits,.zed
	.dc.a 0

	.section .bar,"ao",%progbits,.foo
	.dc.a 0

	.section .zed,"ao",%progbits,.foo
	.dc.a 0
