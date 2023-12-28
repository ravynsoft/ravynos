	.text
	.global bar
	.type	bar, %function
bar:
	.space	16
	.size	bar, 16

	.global bar1
	.type	bar1, %function
bar1:
	.space	8
	.size	bar1, 8

	.symver	bar, foo@FOO
	.symver	bar1, foo@@FOO1
